/*

Copyright 2021 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#include <WiFi.h>
#include <esp_wifi.h>

#include "config.h"
#include "ota.h"
#include "main.h"
#include "webserverc.h"

void WiFiEvent(WiFiEvent_t event)
{
    switch (event)
    {

    case SYSTEM_EVENT_AP_START:
        //can set ap hostname here
        WiFi.softAPsetHostname(otahostname);
        //enable ap ipv6 here
        WiFi.softAPenableIpV6();
        break;

    case SYSTEM_EVENT_STA_START:
        //set sta hostname here
        WiFi.setHostname(otahostname);
        break;
    case SYSTEM_EVENT_STA_CONNECTED:
        //enable sta ipv6 here
        Serial.println(F("WiFi connected to: "));
        Serial.println(WiFi.SSID());

        WiFi.enableIpV6();
        break;
    case SYSTEM_EVENT_AP_STA_GOT_IP6:
        //both interfaces get the same event
        Serial.println("STA IPv6: ");
        Serial.println(WiFi.localIPv6().toString());
        //Serial.println("AP IPv6: ");
        //Serial.println(WiFi.softAPIPv6().toString());
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("STA IPv4: ");
        Serial.println(WiFi.localIP().toString());
        OTASetup();
        WebservercOnWiFiConnect();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println(F("WiFi got disconnected."));
        break;
    default:
        break;
    }
}

void WificSetup()
{
    WiFi.mode(WIFI_STA);

    //esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
    esp_wifi_set_ps(WIFI_PS_NONE);

    Serial.println("WiFi settings:");
    Serial.println(ssid);
    Serial.println(password);
    WiFi.setHostname(otahostname);
    WiFi.begin(ssid, password);

    WiFi.onEvent(WiFiEvent);
}

void WiFicLoop()
{
    OTALoop();
}