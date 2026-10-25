/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include <WiFi.h>
#include <esp_wifi.h>

#include "config.h"
#include "main.h"
#include "ota.h"
#include "webserverc.h"

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
            Serial.println(F("WiFi connected to: "));
            Serial.println(WiFi.SSID());

            WiFi.enableIPv6();
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
            Serial.println("STA IPv6: ");
            Serial.println(WiFi.linkLocalIPv6().toString());
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.println("STA IPv4: ");
            Serial.println(WiFi.localIP().toString());
            OTASetup();
            WebservercOnWiFiConnect();
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.println(F("WiFi got disconnected."));
            break;
        default:
            break;
    }
}

void WificSetup() {
    WiFi.mode(WIFI_STA);

    // esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
    esp_wifi_set_ps(WIFI_PS_NONE);

    Serial.println("WiFi settings:");
    Serial.println(ssid);
    Serial.println(password);
    WiFi.setHostname(otahostname);
    WiFi.begin(ssid, password);

    WiFi.onEvent(WiFiEvent);
}

void WiFicLoop() {
    OTALoop();
}