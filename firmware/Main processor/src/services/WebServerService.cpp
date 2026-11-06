/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "WebServerService.h"

#include <Arduino.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <Update.h>
#include <WiFi.h>

#include <cstring>
#include <memory>
#include <vector>

#include "DeviceName.h"
#include "EventLog.h"
#include "HeatController.h"
#include "HomeWiFiService.h"
#include "Log.h"
#include "WebConfigStore.h"
#include "WebContent.h"
#include "display.h"

#ifndef PRODUCT_VERSION
#define PRODUCT_VERSION "unknown"
#endif

#ifndef GIT_COMMIT
#define GIT_COMMIT "unknown"
#endif

static const char *tag = "Web";

namespace {
    constexpr size_t MAX_POST_BODY = 4096;

    AsyncWebServer server(80);
    AsyncEventSource *eventStream = nullptr;
    DNSServer *dnsServer = nullptr;

    bool serverRunning = false;
    volatile bool portal = false;
    volatile bool portalStopRequested = false;

    uint32_t requestCountValue = 0;
    char lastUriValue[32] = {0};

    char apSsidValue[32] = {0};
    char apPassValue[web_config::AP_PASS_LEN] = {0};
    char apIpValue[16] = {0};

    char stateJson[768] = {0};
    uint32_t lastStreamMs = 0;

    bool otaRunning = false;
    uint8_t otaProgressValue = 0;
    int otaErrorValue = 0;
    uint32_t otaRebootAtMs = 0;

    // Count a request and remember its URI.
    void noteRequest(AsyncWebServerRequest *request) {
        requestCountValue++;
        snprintf(lastUriValue, sizeof(lastUriValue), "%s", request->url().c_str());
    }

    // Build the full state message that the page receives every second.
    void buildStateJson(void) {
        bool linked = HeatController.isCompatible();
        bool heater = HeatController.getHeaterState() == HEATER_STATE::ON;
        String ipText = portal ? WiFi.softAPIP().toString() : WiFi.localIP().toString();

        snprintf(stateJson, sizeof(stateJson),
                 "{\"tip\":%.1f,\"set\":%.1f,\"ntc\":%.1f,\"duty\":%.1f,"
                 "\"vin\":%.2f,\"current\":%.2f,"
                 "\"heater\":%s,\"linked\":%s,\"wifi\":\"%s\",\"ssid\":\"%s\","
                 "\"ip\":\"%s\",\"rssi\":%d,\"portal\":%s,\"uptime\":%lu,\"heap\":%u}",
                 HeatController.getTemperatureTip() - 273.15f,
                 HeatController.getTargetTemperature() - 273.15f,
                 HeatController.getTemperatureNTC() - 273.15f,
                 HeatController.getHeaterDuty(),
                 HeatController.getSupplyVoltage(),
                 HeatController.getIronCurrent(),
                 heater ? "true" : "false",
                 linked ? "true" : "false",
                 home_wifi::stateName(home_wifi::getState()),
                 WiFi.SSID().c_str(),
                 ipText.c_str(),
                 WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0,
                 portal ? "true" : "false",
                 static_cast<unsigned long>(millis() / 1000U),
                 static_cast<unsigned int>(ESP.getFreeHeap()));
    }

    // Serve one gzipped asset from PROGMEM.
    void serveAsset(const char *path, const uint8_t *data, size_t length, const char *contentType) {
        server.on(path, HTTP_GET, [data, length, contentType](AsyncWebServerRequest *request) {
            noteRequest(request);
            AsyncWebServerResponse *response = request->beginResponse(200, contentType, data, length);
            response->addHeader("Content-Encoding", "gzip");
            response->addHeader("Cache-Control", "no-cache");
            request->send(response);
        });
    }

    // Accumulate a POST body into request->_tempObject.
    void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        if (index == 0) {
            request->_tempObject = (total < MAX_POST_BODY) ? calloc(total + 1, sizeof(uint8_t)) : nullptr;
        }

        auto *body = static_cast<char *>(request->_tempObject);
        if (body == nullptr || index + len > total) {
            return;
        }

        memcpy(body + index, data, len);
    }

    // Save the network array and stop the portal so the device reconnects.
    void handleWifiPost(AsyncWebServerRequest *request) {
        noteRequest(request);

        if (!portal) {
            request->send(403, "application/json", "{\"ok\":false,\"error\":\"portal not active\"}");
            return;
        }

        const auto *body = static_cast<const char *>(request->_tempObject);
        if (body == nullptr || body[0] == '\0') {
            request->send(400, "application/json", "{\"ok\":false,\"error\":\"missing body\"}");
            return;
        }

        char sanitized[1024];
        int count = web_config::sanitizeNetworksBody(body, sanitized, sizeof(sanitized));
        if (count <= 0) {
            request->send(400, "application/json", "{\"ok\":false,\"error\":\"no valid networks\"}");
            return;
        }

        if (!web_config::saveNetworksJson(sanitized)) {
            request->send(500, "application/json", "{\"ok\":false,\"error\":\"nvs write failed\"}");
            return;
        }

        LOG_I(tag, "saved %d network(s)", count);
        request->send(200, "application/json", "{\"ok\":true}");
        portalStopRequested = true;
    }

    // Stage one firmware chunk into the update partition.
    void handleOtaBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        (void)request;

        if (!portal) {
            return;
        }

        if (index == 0) {
            otaRunning = true;
            otaProgressValue = 0;
            otaErrorValue = 0;

            if (!Update.begin(total, U_FLASH)) {
                otaErrorValue = Update.getError();
                otaRunning = false;
                return;
            }

            eventlog::Log.record(eventlog::OTA_START, 0);
        }

        if (!otaRunning) {
            return;
        }

        if (Update.write(data, len) != len) {
            otaErrorValue = Update.getError();
            otaRunning = false;
            Update.abort();
            return;
        }

        if (total > 0) {
            otaProgressValue = static_cast<uint8_t>((index + len) * 100U / total);
        }
    }

    // Finish the firmware upload and schedule a reboot.
    void handleOtaPost(AsyncWebServerRequest *request) {
        noteRequest(request);

        if (!portal) {
            request->send(403, "text/plain", "Portal not active");
            return;
        }

        if (!otaRunning || Update.hasError()) {
            otaErrorValue = Update.getError();
            Update.abort();
            otaRunning = false;
            eventlog::Log.record(eventlog::OTA_ERROR, otaErrorValue);
            request->send(500, "text/plain", "Firmware upload failed");
            return;
        }

        if (!Update.end(true)) {
            otaErrorValue = Update.getError();
            otaRunning = false;
            eventlog::Log.record(eventlog::OTA_ERROR, otaErrorValue);
            request->send(500, "text/plain", "Firmware upload failed");
            return;
        }

        otaRunning = false;
        otaProgressValue = 100;
        eventlog::Log.record(eventlog::OTA_END, 0);
        request->send(200, "text/plain", "Firmware upload success, rebooting...");
        otaRebootAtMs = millis() + 2000;
    }

    // Release the panel when the response is destroyed, also on a client
    // disconnect before the last strip was sent.
    struct ScreenshotLock {
        ~ScreenshotLock() {
            DisplayScreenshotEnd();
        }
    };

    // Stream the panel as raw RGB565, one strip at a time. The client turns it
    // into a PNG, so the device never needs a full framebuffer.
    void handleScreenshot(AsyncWebServerRequest *request) {
        noteRequest(request);

        constexpr uint16_t STRIP_H = 16;
        const uint16_t width = DisplayWidth();
        const uint16_t height = DisplayHeight();

        if (width == 0 || height == 0) {
            request->send(503, "text/plain", "Display not ready");
            return;
        }

        if (!DisplayScreenshotBegin(3000)) {
            request->send(503, "text/plain", "Display busy");
            return;
        }

        auto buffer = std::make_shared<std::vector<uint16_t>>(static_cast<size_t>(width) * STRIP_H);
        if (!buffer || buffer->empty()) {
            DisplayScreenshotEnd();
            request->send(500, "text/plain", "Out of memory");
            return;
        }

        auto lock = std::make_shared<ScreenshotLock>();

        AsyncWebServerResponse *response = request->beginChunkedResponse(
            "application/octet-stream",
            [buffer, lock, width, height, stripIndex = uint16_t{0}, byteOff = size_t{0}, stripBytes = size_t{0}](uint8_t *chunk, size_t maxLen, size_t filled) mutable -> size_t {
                (void)filled;
                (void)lock;

                if (stripIndex >= (height + STRIP_H - 1) / STRIP_H) {
                    return 0;
                }

                if (byteOff == 0) {
                    uint16_t y = stripIndex * STRIP_H;
                    uint16_t h = (y + STRIP_H <= height) ? STRIP_H : static_cast<uint16_t>(height - y);
                    DisplayScreenshotRead(y, h, buffer->data());
                    stripBytes = static_cast<size_t>(width) * h * 2;
                }

                size_t remain = stripBytes - byteOff;
                size_t copy = (maxLen < remain) ? maxLen : remain;
                memcpy(chunk, reinterpret_cast<const uint8_t *>(buffer->data()) + byteOff, copy);
                byteOff += copy;

                if (byteOff >= stripBytes) {
                    byteOff = 0;
                    stripIndex++;
                }

                return copy;
            });

        response->addHeader("X-Screenshot-Width", String(width));
        response->addHeader("X-Screenshot-Height", String(height));
        response->addHeader("X-Screenshot-Format", "RGB565");
        response->addHeader("Cache-Control", "no-cache");
        request->send(response);
    }

    // Register every route.
    void registerRoutes(void) {
        serveAsset("/", WEBUI_HTML_GZ, WEBUI_HTML_GZ_LEN, "text/html");
        serveAsset("/style.css", WEBUI_CSS_GZ, WEBUI_CSS_GZ_LEN, "text/css");
        serveAsset("/app.js", WEBUI_JS_GZ, WEBUI_JS_GZ_LEN, "application/javascript");

        server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
            noteRequest(request);

            String ipText = portal ? WiFi.softAPIP().toString() : WiFi.localIP().toString();
            char json[384];
            snprintf(json, sizeof(json),
                     "{\"wifi\":\"%s\",\"ssid\":\"%s\",\"ip\":\"%s\",\"rssi\":%d,"
                     "\"portal\":%s,\"apSsid\":\"%s\",\"apIp\":\"%s\","
                     "\"uptime\":%lu,\"heap\":%u}",
                     home_wifi::stateName(home_wifi::getState()),
                     WiFi.SSID().c_str(),
                     ipText.c_str(),
                     WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0,
                     portal ? "true" : "false",
                     apSsidValue,
                     apIpValue,
                     static_cast<unsigned long>(millis() / 1000U),
                     static_cast<unsigned int>(ESP.getFreeHeap()));
            request->send(200, "application/json", json);
        });

        server.on("/api/device", HTTP_GET, [](AsyncWebServerRequest *request) {
            noteRequest(request);

            char json[320];
            snprintf(json, sizeof(json),
                     "{\"name\":\"%s\",\"version\":\"%s\",\"git\":\"%s\",\"build\":\"%s\","
                     "\"board\":\"%s\",\"chip\":\"%s\"}",
                     DeviceNameGet(), PRODUCT_VERSION, GIT_COMMIT, __DATE__ " " __TIME__,
                     "esp32dev", ESP.getChipModel());
            request->send(200, "application/json", json);
        });

        server.on("/api/config", HTTP_GET, [](AsyncWebServerRequest *request) {
            noteRequest(request);

            String json = "{\"networks\":[";
            char escaped[web_config::SSID_LEN * 6];

            for (size_t i = 0; i < home_wifi::networkCount(); i++) {
                if (i > 0) {
                    json += ",";
                }
                web_config::escapeJson(home_wifi::networks()[i].ssid, escaped, sizeof(escaped));
                json += "{\"ssid\":\"" + String(escaped) + "\"}";
            }

            json += "]}";
            request->send(200, "application/json", json);
        });

        server.on("/api/screenshot", HTTP_GET, [](AsyncWebServerRequest *request) { handleScreenshot(request); });

        server.on("/api/wifi", HTTP_POST, [](AsyncWebServerRequest *request) { handleWifiPost(request); }, nullptr, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
                      if (portal) {
                          handleBody(request, data, len, index, total);
                      } });

        server.on("/api/ota", HTTP_POST, [](AsyncWebServerRequest *request) { handleOtaPost(request); }, nullptr, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) { handleOtaBody(request, data, len, index, total); });

        server.onNotFound([](AsyncWebServerRequest *request) {
            noteRequest(request);

            if (portal) {
                request->redirect("http://" + String(apIpValue) + "/");
            } else {
                request->send(404, "text/plain", "Not found");
            }
        });
    }
}

namespace web_server {
    void setup(void) {
        snprintf(apSsidValue, sizeof(apSsidValue), "%s", DeviceNameGet());
        web_config::ensureApPassword(apPassValue, sizeof(apPassValue));

        buildStateJson();

        registerRoutes();

        eventStream = new AsyncEventSource("/api/stream");
        eventStream->onConnect([](AsyncEventSourceClient *client) {
            client->send(stateJson, "state", millis(), 1000);
        });
        server.addHandler(eventStream);

        server.begin();
        serverRunning = true;

        LOG_I(tag, "server started, AP SSID %s", apSsidValue);
    }

    void loop(void) {
        if (dnsServer != nullptr) {
            dnsServer->processNextRequest();
        }

        if (portalStopRequested) {
            portalStopRequested = false;
            stopPortal();
            home_wifi::reload();
            home_wifi::request();
        }

        if (otaRebootAtMs != 0 && static_cast<int32_t>(millis() - otaRebootAtMs) >= 0) {
            ESP.restart();
        }

        serviceStream();
    }

    bool startPortal(void) {
        if (portal) {
            return true;
        }

        if (apPassValue[0] == '\0') {
            LOG_E(tag, "AP password unavailable");
            return false;
        }

        home_wifi::pause();
        WiFi.mode(WIFI_AP);

        if (!WiFi.softAP(apSsidValue, apPassValue)) {
            LOG_E(tag, "softAP failed");
            WiFi.mode(WIFI_STA);
            home_wifi::resume();
            return false;
        }

        snprintf(apIpValue, sizeof(apIpValue), "%s", WiFi.softAPIP().toString().c_str());

        dnsServer = new DNSServer();
        dnsServer->start(53, "*", WiFi.softAPIP());

        portal = true;
        LOG_I(tag, "portal %s at %s", apSsidValue, apIpValue);
        return true;
    }

    void stopPortal(void) {
        if (!portal) {
            return;
        }

        if (dnsServer != nullptr) {
            dnsServer->stop();
            delete dnsServer;
            dnsServer = nullptr;
        }

        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_STA);

        portal = false;
        apIpValue[0] = '\0';
        home_wifi::resume();

        LOG_I(tag, "portal stopped");
    }

    bool portalActive(void) {
        return portal;
    }

    bool running(void) {
        return serverRunning;
    }

    uint32_t requestCount(void) {
        return requestCountValue;
    }

    const char *lastUri(void) {
        return lastUriValue;
    }

    const char *apSsid(void) {
        return apSsidValue;
    }

    const char *apPassword(void) {
        return apPassValue;
    }

    const char *apIp(void) {
        return apIpValue;
    }

    void serviceStream(void) {
        uint32_t now = millis();

        if (now - lastStreamMs < 1000) {
            return;
        }
        lastStreamMs = now;

        buildStateJson();

        if (eventStream != nullptr) {
            eventStream->send(stateJson, "state", now);
        }
    }

    bool otaActive(void) {
        return otaRunning;
    }

    uint8_t otaProgress(void) {
        return otaProgressValue;
    }

    int otaError(void) {
        return otaErrorValue;
    }
}
