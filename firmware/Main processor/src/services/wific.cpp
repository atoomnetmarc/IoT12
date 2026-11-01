/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include <WiFi.h>
#include <esp_wifi.h>

#include "EventLog.h"
#include "Log.h"
#include "config.h"
#include "ota.h"
#include "webserverc.h"
#include "wific.h"

static const char *tag = "WiFi";

namespace {
    uint32_t connectCount = 0;
    uint32_t disconnectCount = 0;

    /*
     * The ESP32 retries a failed association every ~2.4 s and raises a
     * DISCONNECTED event each time. Logging every attempt produced a
     * periodic print, which is exactly what this milestone removes.
     * Only a real connected -> disconnected transition is an event.
     */
    bool wasConnected = false;
    wl_status_t lastStatus = WL_IDLE_STATUS;
}

void WiFiEvent(WiFiEvent_t event) {
    switch (event) {

        case ARDUINO_EVENT_WIFI_AP_START:
            // can set ap hostname here
            WiFi.softAPsetHostname(otahostname);
            // enable ap ipv6 here
            WiFi.softAPenableIPv6();
            break;

        case ARDUINO_EVENT_WIFI_STA_START:
            // set sta hostname here
            WiFi.setHostname(otahostname);
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            // enable sta ipv6 here
            WiFi.enableIPv6();
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            connectCount++;
            wasConnected = true;
            lastStatus = WiFi.status();
            eventlog::Log.record(eventlog::WIFI_CONNECT, static_cast<int32_t>(WiFi.RSSI()));
            LOG_I(tag, "connected to %s, ip %s, rssi %d", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str(), WiFi.RSSI());
            OTASetup();
            WebservercOnWiFiConnect();
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED: {
            disconnectCount++;
            wl_status_t status = WiFi.status();

            if (wasConnected) {
                /* Lost a working link: this is a real event. */
                wasConnected = false;
                eventlog::Log.record(eventlog::WIFI_DISCONNECT, static_cast<int32_t>(status));
                LOG_W(tag, "disconnected, status %d", static_cast<int>(status));
            } else if (status != lastStatus) {
                /* Still trying to associate: detail only, no event spam. */
                LOG_D(tag, "connect attempt failed, status %d", static_cast<int>(status));
            }

            lastStatus = status;
            break;
        }
        default:
            break;
    }
}

void WificSetup() {
    WiFi.mode(WIFI_STA);

    // esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
    esp_wifi_set_ps(WIFI_PS_NONE);

    WiFi.setHostname(otahostname);
    WiFi.begin(ssid, password);

    WiFi.onEvent(WiFiEvent);
}

void WiFicLoop() {
    OTALoop();
}

uint32_t WificConnectCount(void) {
    return connectCount;
}

uint32_t WificDisconnectCount(void) {
    return disconnectCount;
}
