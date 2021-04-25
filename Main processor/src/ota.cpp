/*

Copyright 2021 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#include <ArduinoOTA.h>

#include "config.h"
#include "main.h"

void OTASetup(void)
{
    ArduinoOTA.setPort(3232);

    // Hostname defaults to esp3232-[MAC]
    ArduinoOTA.setHostname(otahostname);
    ArduinoOTA.setPassword(otapassword);

    ArduinoOTA
        .onStart([]() {
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH)
                type = "sketch";
            else // U_SPIFFS
                type = "filesystem";

            // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
            Serial.println("Start updating " + type);
        })
        .onEnd([]() {
            Serial.println("\nEnd");
        })
        .onProgress([](unsigned int progress, unsigned int total) {

            static uint8_t previousprogress = 255;

            uint8_t newprogress = (progress / (total / 100));

            if (previousprogress != newprogress)
            {
                Serial.printf("Progress: %u%%\r", newprogress);
                previousprogress = newprogress;
            }
        })
        .onError([](ota_error_t error) {
            Serial.printf("Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR)
                Serial.println("Auth Failed");
            else if (error == OTA_BEGIN_ERROR)
                Serial.println("Begin Failed");
            else if (error == OTA_CONNECT_ERROR)
                Serial.println("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR)
                Serial.println("Receive Failed");
            else if (error == OTA_END_ERROR)
                Serial.println("End Failed");
        });

    ArduinoOTA.begin();

    Serial.println("Ready");
}

void OTALoop()
{
    ArduinoOTA.handle();
}