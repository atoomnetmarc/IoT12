/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "HomeWiFiService.h"

#include <Arduino.h>
#include <WiFi.h>

#include "DeviceName.h"
#include "EventLog.h"
#include "Log.h"
#include "MdnsService.h"

static const char *tag = "WiFi";

namespace {
    constexpr uint32_t CONNECT_TIMEOUT_MS = 20000;
    constexpr uint32_t RETRY_DELAY_MS = 30000;

    web_config::Network networkList[web_config::MAX_NETWORKS];
    size_t networkCountValue = 0;

    home_wifi::State state = home_wifi::State::IDLE;
    bool requestPending = false;
    bool suppressed = false;
    uint32_t attemptStartMs = 0;
    uint32_t retryAtMs = 0;
    uint32_t connectCountValue = 0;
    uint32_t disconnectCountValue = 0;

    void setState(home_wifi::State next) {
        if (state == next) {
            return;
        }

        state = next;
        LOG_I(tag, "state %s", home_wifi::stateName(next));
    }

    // Start a passive scan; the result is polled in loop().
    void startScan(void) {
        WiFi.scanNetworks(true, false, true, 120);
        setState(home_wifi::State::SCANNING);
    }

    // Pick the strongest configured network from the scan result and connect.
    void connectBest(void) {
        int found = WiFi.scanComplete();

        if (found < 0) {
            WiFi.scanDelete();
            setState(home_wifi::State::IDLE);
            retryAtMs = millis() + RETRY_DELAY_MS;
            return;
        }

        int bestSlot = -1;
        int32_t bestRssi = INT32_MIN;

        for (int i = 0; i < found; i++) {
            String ssid = WiFi.SSID(i);

            for (size_t slot = 0; slot < networkCountValue; slot++) {
                if (ssid == networkList[slot].ssid && WiFi.RSSI(i) > bestRssi) {
                    bestSlot = static_cast<int>(slot);
                    bestRssi = WiFi.RSSI(i);
                }
            }
        }

        WiFi.scanDelete();

        if (bestSlot < 0) {
            LOG_W(tag, "none of the %u configured networks in range", static_cast<unsigned>(networkCountValue));
            setState(home_wifi::State::IDLE);
            retryAtMs = millis() + RETRY_DELAY_MS;
            return;
        }

        LOG_I(tag, "connecting to %s, rssi %ld", networkList[bestSlot].ssid, static_cast<long>(bestRssi));
        WiFi.begin(networkList[bestSlot].ssid, networkList[bestSlot].pass);
        attemptStartMs = millis();
        setState(home_wifi::State::CONNECTING);
    }
}

namespace home_wifi {
    void setup(void) {
        networkCountValue = web_config::loadNetworks(networkList, web_config::MAX_NETWORKS);

        WiFi.mode(WIFI_STA);
        WiFi.setHostname(DeviceNameGet());
        WiFi.setAutoReconnect(false);
        WiFi.setSleep(false);

        requestPending = networkCountValue > 0;
        retryAtMs = 0;

        LOG_I(tag, "service started, %u network(s)", static_cast<unsigned>(networkCountValue));
    }

    void loop(void) {
        if (suppressed) {
            return;
        }

        uint32_t now = millis();

        switch (state) {
            case State::IDLE:
                if (requestPending && networkCountValue > 0 && static_cast<int32_t>(now - retryAtMs) >= 0) {
                    requestPending = false;
                    startScan();
                }
                break;

            case State::SCANNING:
                if (WiFi.scanComplete() != WIFI_SCAN_RUNNING) {
                    connectBest();
                }
                break;

            case State::CONNECTING:
                if (WiFi.status() == WL_CONNECTED) {
                    connectCountValue++;
                    setState(State::CONNECTED);
                    eventlog::Log.record(eventlog::WIFI_CONNECT, WiFi.RSSI());
                    LOG_I(tag, "connected to %s, ip %s, rssi %d", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str(), WiFi.RSSI());
                    mdns::start();
                } else if (now - attemptStartMs >= CONNECT_TIMEOUT_MS) {
                    LOG_W(tag, "connect attempt timed out");
                    WiFi.disconnect(false);
                    setState(State::IDLE);
                    retryAtMs = now + RETRY_DELAY_MS;
                }
                break;

            case State::CONNECTED:
                if (WiFi.status() != WL_CONNECTED) {
                    disconnectCountValue++;
                    setState(State::IDLE);
                    requestPending = true;
                    retryAtMs = now + 1000;
                    eventlog::Log.record(eventlog::WIFI_DISCONNECT, static_cast<int32_t>(WiFi.status()));
                    LOG_W(tag, "disconnected, status %d", static_cast<int>(WiFi.status()));
                    mdns::stop();
                }
                break;
        }
    }

    void request(void) {
        requestPending = true;
        retryAtMs = 0;
    }

    void reload(void) {
        networkCountValue = web_config::loadNetworks(networkList, web_config::MAX_NETWORKS);
        requestPending = networkCountValue > 0;
        retryAtMs = 0;
    }

    State getState(void) {
        return state;
    }

    const char *stateName(State value) {
        switch (value) {
            case State::IDLE:
                return "idle";
            case State::SCANNING:
                return "scanning";
            case State::CONNECTING:
                return "connecting";
            case State::CONNECTED:
                return "connected";
            default:
                return "unknown";
        }
    }

    bool hasNetworks(void) {
        return networkCountValue > 0;
    }

    size_t networkCount(void) {
        return networkCountValue;
    }

    const web_config::Network *networks(void) {
        return networkList;
    }

    void pause(void) {
        suppressed = true;
        requestPending = false;

        // The responder holds the station netif, which the AP mode switch destroys.
        mdns::stop();

        if (state != State::IDLE) {
            WiFi.disconnect(false);
            setState(State::IDLE);
        }

        LOG_I(tag, "paused for the portal");
    }

    void resume(void) {
        suppressed = false;
        requestPending = true;
        retryAtMs = 0;
        LOG_I(tag, "resumed");
    }

    bool isSuppressed(void) {
        return suppressed;
    }

    uint32_t connectCount(void) {
        return connectCountValue;
    }

    uint32_t disconnectCount(void) {
        return disconnectCountValue;
    }
}
