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
#include <esp_mac.h>
#include <esp_system.h>

#include <cmath>
#include <cstdarg>
#include <cstring>
#include <memory>
#include <vector>

#include "DeviceName.h"
#include "EventLog.h"
#include "GitCommit.h"
#include "HandleDiagnosis.h"
#include "HeatController.h"
#include "HomeWiFiService.h"
#include "LifetimeStats.h"
#include "Log.h"
#include "SourceBundle.h"
#include "SystemCommands.h"
#include "Temperature.h"
#include "TemperatureUnit.h"
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
    bool wifiReloadRequested = false;

    constexpr const char *SESSION_COOKIE = "iot12_session";
    constexpr size_t SESSION_TOKEN_LEN = 32;

    char sessionToken[SESSION_TOKEN_LEN + 1] = {0};
    bool sessionTokenValid = false;

    uint32_t requestCountValue = 0;
    char lastUriValue[32] = {0};

    char apSsidValue[32] = {0};
    char apPassValue[web_config::AP_PASS_LEN] = {0};
    char apIpValue[16] = {0};

    char stateJson[2048] = {0};
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

    // Generate a new random session token.
    void generateSessionToken(void) {
        for (size_t i = 0; i < SESSION_TOKEN_LEN; i += 8) {
            snprintf(sessionToken + i, SESSION_TOKEN_LEN + 1 - i, "%08x", static_cast<unsigned int>(esp_random()));
        }
        sessionTokenValid = true;
    }

    // Compare a candidate token with the session token in constant time.
    bool tokenMatches(const char *candidate) {
        if (!sessionTokenValid || candidate == nullptr || strlen(candidate) != SESSION_TOKEN_LEN) {
            return false;
        }

        unsigned char diff = 0;
        for (size_t i = 0; i < SESSION_TOKEN_LEN; i++) {
            diff |= static_cast<unsigned char>(candidate[i] ^ sessionToken[i]);
        }
        return diff == 0;
    }

    // Read the session token from the Cookie header.
    bool cookieToken(AsyncWebServerRequest *request, char *out, size_t outSize) {
        const AsyncWebHeader *header = request->getHeader("Cookie");
        if (header == nullptr) {
            return false;
        }

        String cookies = header->value();
        int start = cookies.indexOf(SESSION_COOKIE);

        while (start >= 0) {
            bool atStart = start == 0 || cookies[start - 1] == ';' || cookies[start - 1] == ' ';
            int after = start + strlen(SESSION_COOKIE);
            bool hasEquals = after < static_cast<int>(cookies.length()) && cookies[after] == '=';
            if (atStart && hasEquals) {
                break;
            }
            start = cookies.indexOf(SESSION_COOKIE, start + 1);
        }

        if (start < 0) {
            return false;
        }

        start += strlen(SESSION_COOKIE);
        if (start >= static_cast<int>(cookies.length()) || cookies[start] != '=') {
            return false;
        }
        start++;

        int end = cookies.indexOf(';', start);
        String value = end < 0 ? cookies.substring(start) : cookies.substring(start, end);
        value.trim();

        if (value.length() == 0 || value.length() >= outSize) {
            return false;
        }

        snprintf(out, outSize, "%s", value.c_str());
        return true;
    }

    // True when the request may change settings: the portal is open, or the
    // session cookie matches the device token.
    bool isUnlocked(AsyncWebServerRequest *request) {
        if (portal) {
            return true;
        }

        char token[SESSION_TOKEN_LEN + 1];
        if (!cookieToken(request, token, sizeof(token))) {
            return false;
        }

        return tokenMatches(token);
    }

    // Add the session cookie to a response.
    void addSessionCookie(AsyncWebServerResponse *response) {
        response->addHeader("Set-Cookie", String(SESSION_COOKIE) + "=" + sessionToken + "; Path=/; HttpOnly; SameSite=Strict");
    }

    // Expire the session cookie in the browser.
    void addExpiredSessionCookie(AsyncWebServerResponse *response) {
        response->addHeader("Set-Cookie", String(SESSION_COOKIE) + "=; Path=/; HttpOnly; SameSite=Strict; Max-Age=0");
    }

    // Append formatted text to the state message and advance the cursor.
    void appendJson(char *&cursor, size_t &left, const char *format, ...) {
        va_list args;
        va_start(args, format);
        int written = vsnprintf(cursor, left, format, args);
        va_end(args);

        if (written > 0 && static_cast<size_t>(written) < left) {
            cursor += written;
            left -= written;
        }
    }

    // Append a float as a JSON number, or null when it is not finite.
    void appendJsonFloat(char *&cursor, size_t &left, float value, int digits) {
        if (isfinite(value)) {
            appendJson(cursor, left, "%.*f", digits, value);
        } else {
            appendJson(cursor, left, "null");
        }
    }

    // Append a key and a float value to the state message.
    void appendJsonKeyFloat(char *&cursor, size_t &left, const char *key, float value, int digits) {
        appendJson(cursor, left, ",\"%s\":", key);
        appendJsonFloat(cursor, left, value, digits);
    }

    // Append a key and a text value to the state message.
    void appendJsonKeyText(char *&cursor, size_t &left, const char *key, const char *value) {
        appendJson(cursor, left, ",\"%s\":\"%s\"", key, value);
    }

    // Converts a Kelvin value to the active display unit.
    float displayTemperature(float kelvin) {
        return temperature_unit::active()->fromKelvin(kelvin);
    }

    // Build the full state message that the page receives every second.
    void buildStateJson(void) {
        bool heater = HeatController.getHeaterState() == HEATER_STATE::ON;
        String ipText = portal ? WiFi.softAPIP().toString() : WiFi.localIP().toString();
        HandleDiagnosis diagnosis = HandleDiagnosisClassify();

        char *cursor = stateJson;
        size_t left = sizeof(stateJson);

        appendJson(cursor, left, "{\"tip\":");
        appendJsonFloat(cursor, left, displayTemperature(HeatController.getTemperatureTip()), 1);
        appendJsonKeyFloat(cursor, left, "set", displayTemperature(HeatController.getTargetTemperature()), 1);
        appendJsonKeyFloat(cursor, left, "ntc", displayTemperature(HeatController.getTemperatureNTC()), 1);
        appendJsonKeyText(cursor, left, "unit", temperature_unit::id());
        appendJsonKeyText(cursor, left, "unitSymbol", temperature_unit::active()->symbol);
        appendJson(cursor, left, ",\"unitDecimals\":%u", static_cast<unsigned>(temperature_unit::active()->decimals));
        appendJsonKeyFloat(cursor, left, "duty", HeatController.getHeaterDuty(), 1);
        appendJsonKeyFloat(cursor, left, "current", HeatController.getIronCurrent(), 2);
        appendJsonKeyText(cursor, left, "handle", HandleStateName(diagnosis.state));
        appendJsonKeyFloat(cursor, left, "ntc_v", HeatController.getVoltageNTC(), 3);
        appendJsonKeyFloat(cursor, left, "tc_mv", HeatController.getVoltageThermocouple() * 1000.0f, 1);
        appendJsonKeyFloat(cursor, left, "vin_off", HeatController.getSupplyVoltage(), 2);
        appendJsonKeyFloat(cursor, left, "vin_on", HeatController.getVoltageVinOn(), 2);
        appendJsonKeyFloat(cursor, left, "iron_off", HeatController.getCurrentIronOff(), 3);
        appendJsonKeyFloat(cursor, left, "iron_on", HeatController.getCurrentIronOn(), 3);
        appendJsonKeyFloat(cursor, left, "rtip", HeatController.getResistanceTip(), 2);
        appendJsonKeyFloat(cursor, left, "rsupply", HeatController.getResistanceSupply(), 2);
        appendJson(cursor, left, ",\"heater\":%s", heater ? "true" : "false");
        appendJson(cursor, left, ",\"linked\":%s", HeatController.linkUp() ? "true" : "false");
        appendJsonKeyText(cursor, left, "wifi", home_wifi::stateName(home_wifi::getState()));
        appendJsonKeyText(cursor, left, "ssid", WiFi.SSID().c_str());
        appendJsonKeyText(cursor, left, "ip", ipText.c_str());
        appendJson(cursor, left, ",\"rssi\":%d", WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0);
        appendJson(cursor, left, ",\"portal\":%s", portal ? "true" : "false");

        appendJson(cursor, left, ",\"uptime\":%lu", static_cast<unsigned long>(lifetime_stats::sessionSeconds()));
        appendJson(cursor, left, ",\"total\":%lu", static_cast<unsigned long>(lifetime_stats::totalSeconds()));
        appendJson(cursor, left, ",\"boots\":%lu", static_cast<unsigned long>(lifetime_stats::bootCount()));
        appendJsonKeyText(cursor, left, "reset", system_commands::resetReasonName(esp_reset_reason()));
        appendJson(cursor, left, ",\"heap\":%u", static_cast<unsigned int>(ESP.getFreeHeap()));
        appendJson(cursor, left, ",\"heap_min\":%u", static_cast<unsigned int>(ESP.getMinFreeHeap()));
        appendJson(cursor, left, ",\"heap_max\":%u", static_cast<unsigned int>(ESP.getMaxAllocHeap()));
        appendJson(cursor, left, ",\"heap_size\":%u", static_cast<unsigned int>(ESP.getHeapSize()));
        appendJsonKeyFloat(cursor, left, "die", displayTemperature(temperatureRead() + 273.15f), 1);

        appendJson(cursor, left, ",\"minionUptime\":%lu", static_cast<unsigned long>(HeatController.getUptime()));
        appendJsonKeyText(cursor, left, "minionReset", HeatController.getResetCauseName());
        appendJson(cursor, left, ",\"minionFreeRam\":%lu", static_cast<unsigned long>(HeatController.getFreeRam()));
        appendJson(cursor, left, ",\"minionEvents\":%u", static_cast<unsigned int>(HeatController.getEventCount()));
        appendJsonKeyFloat(cursor, left, "minionDie", displayTemperature(HeatController.getDieTemperature() + 273.15f), 1);
        appendJsonKeyFloat(cursor, left, "minionVdd", HeatController.getVdd(), 2);

        appendJson(cursor, left, ",\"linkAge\":%lu", static_cast<unsigned long>(HeatController.linkAgeMs()));
        appendJson(cursor, left, ",\"heartbeats\":%lu", static_cast<unsigned long>(HeatController.heartbeatCount()));
        appendJson(cursor, left, ",\"errors\":%lu", static_cast<unsigned long>(HeatController.errorCount()));
        appendJson(cursor, left, ",\"rewrites\":%lu", static_cast<unsigned long>(HeatController.rewriteCount()));
        appendJson(cursor, left, ",\"forcedOff\":%lu", static_cast<unsigned long>(HeatController.getForcedOffCount()));
        appendJson(cursor, left, ",\"lastInt\":%lu", static_cast<unsigned long>(HeatController.getLastInterrupt()));
        appendJson(cursor, left, ",\"compatible\":%s", HeatController.isCompatible() ? "true" : "false");

        appendJson(cursor, left, ",\"cycles\":%lu", static_cast<unsigned long>(HeatController.getCycleCount()));
        appendJson(cursor, left, ",\"adcMs\":%lu", static_cast<unsigned long>(HeatController.getAdcReadMs()));

        float p, i, d, heatfactor;
        HeatController.getPidTerms(&p, &i, &d, &heatfactor);
        appendJsonKeyFloat(cursor, left, "p", p, 4);
        appendJsonKeyFloat(cursor, left, "i", i, 4);
        appendJsonKeyFloat(cursor, left, "d", d, 4);
        appendJsonKeyFloat(cursor, left, "heatfactor", heatfactor, 4);

        float kp, ki, kd;
        HeatController.getPidGains(&kp, &ki, &kd);
        appendJsonKeyFloat(cursor, left, "kp", kp, 5);
        appendJsonKeyFloat(cursor, left, "ki", ki, 5);
        appendJsonKeyFloat(cursor, left, "kd", kd, 6);

        uint32_t pwmDuty, pwmPeriod;
        HeatController.getPwmRaw(&pwmDuty, &pwmPeriod);
        appendJson(cursor, left, ",\"pwmDuty\":%lu", static_cast<unsigned long>(pwmDuty));
        appendJson(cursor, left, ",\"pwmPeriod\":%lu", static_cast<unsigned long>(pwmPeriod));

        appendJsonKeyFloat(cursor, left, "shakeV", HeatController.getShakeVoltage(), 3);

        uint8_t shakePhase;
        bool shakeRaw, shakeDebounced, shakeExpired;
        HeatController.getShakeState(&shakePhase, &shakeRaw, &shakeDebounced, &shakeExpired);
        appendJson(cursor, left, ",\"shakePhase\":%u", static_cast<unsigned int>(shakePhase));
        appendJson(cursor, left, ",\"shakeRaw\":%s", shakeRaw ? "true" : "false");
        appendJson(cursor, left, ",\"shakeDebounced\":%s", shakeDebounced ? "true" : "false");
        appendJson(cursor, left, ",\"shakeExpired\":%s", shakeExpired ? "true" : "false");
        appendJson(cursor, left, ",\"shakeAge\":%lu", static_cast<unsigned long>(HeatController.getShakeLastMotionAge()));

        appendJson(cursor, left, "}");
    }

    // Returns the soft-AP MAC from eFuse, which is valid before the AP starts.
    String apMacAddress(void) {
        uint8_t mac[6] = {0};
        char text[18] = {0};
        esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP);
        snprintf(text, sizeof(text), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        return String(text);
    }

    // Build the fixed device data that the page fetches after a reboot.
    void buildDeviceJson(char *out, size_t size) {
        char *cursor = out;
        size_t left = size;

        appendJson(cursor, left, "{\"name\":\"%s\",\"version\":\"%s\",\"git\":\"%s\",\"build\":\"%s\",",
                   DeviceNameGet(), PRODUCT_VERSION, GIT_COMMIT, __DATE__ " " __TIME__);
        appendJson(cursor, left, "\"unitSymbol\":\"%s\",\"unitDecimals\":%u,",
                   temperature_unit::active()->symbol, static_cast<unsigned>(temperature_unit::active()->decimals));
        appendJson(cursor, left, "\"board\":\"esp32dev\",\"chip\":\"%s\",\"chipRevision\":%u,",
                   ESP.getChipModel(), static_cast<unsigned>(ESP.getChipRevision()));
        appendJson(cursor, left, "\"cores\":%u,\"cpuMhz\":%u,\"sdk\":\"%s\",",
                   static_cast<unsigned>(ESP.getChipCores()), static_cast<unsigned>(ESP.getCpuFreqMHz()), ESP.getSdkVersion());
        appendJson(cursor, left, "\"flashSize\":%u,\"flashSpeed\":%u,\"flashMode\":%u,",
                   static_cast<unsigned>(ESP.getFlashChipSize()), static_cast<unsigned>(ESP.getFlashChipSpeed()),
                   static_cast<unsigned>(ESP.getFlashChipMode()));
        uint32_t sketchSize = ESP.getSketchSize();
        uint32_t freeSketch = ESP.getFreeSketchSpace() > sketchSize ? ESP.getFreeSketchSpace() - sketchSize : 0;
        appendJson(cursor, left, "\"sketchSize\":%u,\"freeSketch\":%u,",
                   static_cast<unsigned>(sketchSize), static_cast<unsigned>(freeSketch));
        appendJson(cursor, left, "\"mac\":\"%s\",\"apMac\":\"%s\",\"hostname\":\"%s\",",
                   WiFi.macAddress().c_str(), apMacAddress().c_str(), WiFi.getHostname());

        appendJson(cursor, left, "\"minion\":{\"linked\":%s,\"compatible\":%s,",
                   HeatController.linkUp() ? "true" : "false",
                   HeatController.isCompatible() ? "true" : "false");
        appendJson(cursor, left, "\"version\":\"%s\",\"board\":\"%s\",\"build\":\"%s\",",
                   HeatController.getFirmwareVersion(), HeatController.getBoard(), HeatController.getBuild());
        appendJson(cursor, left, "\"sketchSize\":%u,\"freeSketch\":%u,",
                   static_cast<unsigned>(HeatController.getSketchSize()),
                   static_cast<unsigned>(HeatController.getFreeSketch()));

        SerialNumber_t sn = HeatController.getSerialNumber();
        appendJson(cursor, left, "\"serial\":\"%08lx%08lx%08lx%08lx\",",
                   static_cast<unsigned long>(sn.word0), static_cast<unsigned long>(sn.word1),
                   static_cast<unsigned long>(sn.word2), static_cast<unsigned long>(sn.word3));

        appendJson(cursor, left, "\"const1\":%u,\"const2\":%.6f,",
                   static_cast<unsigned>(HeatController.getConst1()), HeatController.getConst2());
        appendJson(cursor, left, "\"linkTimeout\":%lu,",
                   static_cast<unsigned long>(HeatController.getLinkTimeout()));
        appendJson(cursor, left, "\"ntcPullup\":%.2f,\"heaterShunt\":%.4f,",
                   HeatController.getResistanceNtcPullup(), HeatController.getResistanceHeaterShunt());
        appendJson(cursor, left, "\"gainDivider\":%.5f,\"gainTemp\":%.2f,\"gainCurrent\":%.2f,",
                   HeatController.getGainVSupplyResistorDivider(),
                   HeatController.getGainHeaterTemperatureAmplifier(),
                   HeatController.getGainHeaterCurrentAmplifier());
        appendJson(cursor, left, "\"thermocoupleOffset\":%.6f,",
                   HeatController.getThermocoupleOffset());
        appendJson(cursor, left, "\"standby1\":%.2f,\"standby2\":%.2f,",
                   displayTemperature(HeatController.getStandbyTemp1()), displayTemperature(HeatController.getStandbyTemp2()));
        appendJson(cursor, left, "\"shakeStage1\":%.0f,\"shakeStage2\":%.0f,\"shakeOff\":%.0f",
                   HeatController.getShakeStage1Ms(), HeatController.getShakeStage2Ms(),
                   HeatController.getShakeOffMs());
        appendJson(cursor, left, "}}");
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

    // Serve the gzipped source archive as a download.
    void serveSource(void) {
        server.on("/api/source", HTTP_GET, [](AsyncWebServerRequest *request) {
            noteRequest(request);

            char filename[64];
            snprintf(filename, sizeof(filename), "iot12-source-%s-%s.tar.gz", PRODUCT_VERSION, GIT_COMMIT);

            AsyncWebServerResponse *response = request->beginResponse(200, "application/gzip", SOURCE_GZ, SOURCE_GZ_LEN);
            response->addHeader("Content-Disposition", String("attachment; filename=\"") + filename + "\"");
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

    // Save the network array and reconnect.
    void handleWifiPost(AsyncWebServerRequest *request) {
        noteRequest(request);

        if (!isUnlocked(request)) {
            request->send(401, "application/json", "{\"ok\":false,\"error\":\"locked\"}");
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

        if (portal) {
            portalStopRequested = true;
        } else {
            wifiReloadRequested = true;
        }
    }

    // Store the temperature unit id from a JSON body.
    void handleUnitPost(AsyncWebServerRequest *request) {
        noteRequest(request);

        const auto *body = static_cast<const char *>(request->_tempObject);
        if (body == nullptr) {
            request->send(400, "application/json", "{\"ok\":false,\"error\":\"missing body\"}");
            return;
        }

        const char *key = strstr(body, "\"unit\"");
        const char *colon = key != nullptr ? strchr(key, ':') : nullptr;
        const char *quote = colon != nullptr ? strchr(colon, '"') : nullptr;
        const char *end = quote != nullptr ? strchr(quote + 1, '"') : nullptr;

        if (end == nullptr || static_cast<size_t>(end - quote - 1) >= 16) {
            request->send(400, "application/json", "{\"ok\":false,\"error\":\"invalid unit\"}");
            return;
        }

        char id[16];
        size_t length = static_cast<size_t>(end - quote - 1);
        memcpy(id, quote + 1, length);
        id[length] = '\0';

        if (!temperature_unit::set(id)) {
            request->send(400, "application/json", "{\"ok\":false,\"error\":\"unknown unit\"}");
            return;
        }

        DisplayRequestDraw();
        request->send(200, "application/json", "{\"ok\":true}");
    }

    // Stage one firmware chunk into the update partition.
    void handleOtaBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        if (index == 0) {
            if (!isUnlocked(request)) {
                return;
            }

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

        if (!isUnlocked(request)) {
            request->send(401, "text/plain", "Locked");
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

    // Check the password and set the session cookie.
    void handleLoginPost(AsyncWebServerRequest *request) {
        noteRequest(request);

        const auto *body = static_cast<const char *>(request->_tempObject);
        if (apPassValue[0] == '\0' || body == nullptr || strcmp(body, apPassValue) != 0) {
            request->send(401, "application/json", "{\"ok\":false,\"error\":\"wrong password\"}");
            return;
        }

        if (!sessionTokenValid) {
            generateSessionToken();
        }

        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"ok\":true}");
        addSessionCookie(response);
        request->send(response);
    }

    // Clear the session cookie in the browser; the device token stays valid.
    void handleLogoutPost(AsyncWebServerRequest *request) {
        noteRequest(request);

        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"ok\":true}");
        addExpiredSessionCookie(response);
        request->send(response);
    }

    // Report whether the request carries a valid session cookie.
    void handleSessionGet(AsyncWebServerRequest *request) {
        noteRequest(request);

        char json[32];
        snprintf(json, sizeof(json), "{\"unlocked\":%s}", isUnlocked(request) ? "true" : "false");
        request->send(200, "application/json", json);
    }

    // Replace the AP password, rotate the session token and return the new password.
    void handleRegeneratePost(AsyncWebServerRequest *request) {
        noteRequest(request);

        if (!isUnlocked(request)) {
            request->send(401, "application/json", "{\"ok\":false,\"error\":\"locked\"}");
            return;
        }

        if (portal) {
            request->send(403, "application/json", "{\"ok\":false,\"error\":\"portal active\"}");
            return;
        }

        char newPass[web_config::AP_PASS_LEN];
        web_config::generateApPassword(newPass, sizeof(newPass));

        if (!web_config::saveApPassword(newPass)) {
            request->send(500, "application/json", "{\"ok\":false,\"error\":\"nvs write failed\"}");
            return;
        }

        snprintf(apPassValue, sizeof(apPassValue), "%s", newPass);
        generateSessionToken();

        char json[64];
        snprintf(json, sizeof(json), "{\"ok\":true,\"password\":\"%s\"}", newPass);

        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
        addSessionCookie(response);
        request->send(response);

        LOG_I(tag, "AP password regenerated");
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
        serveSource();

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

            char json[2048];
            buildDeviceJson(json, sizeof(json));
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

            json += "],\"temperatureUnit\":\"";
            json += temperature_unit::id();
            json += "\",\"temperatureUnits\":[";

            for (size_t i = 0; i < temperature::unitCount(); i++) {
                const temperature::Unit *unit = &temperature::units()[i];
                if (i > 0) {
                    json += ",";
                }
                json += "{\"id\":\"";
                json += unit->id;
                json += "\",\"name\":\"";
                json += unit->name;
                json += "\",\"symbol\":\"";
                json += unit->symbol;
                json += "\",\"decimals\":";
                json += String(unit->decimals);
                json += ",\"common\":";
                json += unit->common ? "true" : "false";
                json += "}";
            }

            json += "]}";
            request->send(200, "application/json", json);
        });

        server.on("/api/screenshot", HTTP_GET, [](AsyncWebServerRequest *request) { handleScreenshot(request); });

        server.on("/api/session", HTTP_GET, [](AsyncWebServerRequest *request) { handleSessionGet(request); });

        server.on("/api/login", HTTP_POST, [](AsyncWebServerRequest *request) { handleLoginPost(request); }, nullptr, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) { handleBody(request, data, len, index, total); });

        server.on("/api/logout", HTTP_POST, [](AsyncWebServerRequest *request) { handleLogoutPost(request); });

        server.on("/api/regenerate", HTTP_POST, [](AsyncWebServerRequest *request) { handleRegeneratePost(request); });

        server.on("/api/wifi", HTTP_POST, [](AsyncWebServerRequest *request) { handleWifiPost(request); }, nullptr, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
                      if (isUnlocked(request)) {
                          handleBody(request, data, len, index, total);
                      } });

        server.on("/api/unit", HTTP_POST, [](AsyncWebServerRequest *request) { handleUnitPost(request); }, nullptr, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) { handleBody(request, data, len, index, total); });

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

        if (wifiReloadRequested) {
            wifiReloadRequested = false;
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
