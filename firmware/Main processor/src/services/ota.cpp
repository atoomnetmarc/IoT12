/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include <ArduinoOTA.h>

#include "EventLog.h"
#include "Log.h"
#include "config.h"
#include "ota.h"

static const char *tag = "OTA";

namespace {
    OtaState state = OTA_STATE_IDLE;
    int lastError = 0;
    uint8_t progress = 0;
}

void OTASetup(void) {
    ArduinoOTA.setPort(3232);

    // Hostname defaults to esp3232-[MAC]
    ArduinoOTA.setHostname(otahostname);
    ArduinoOTA.setPassword(otapassword);

    ArduinoOTA
        .onStart([]() {
            state = OTA_STATE_RUNNING;
            progress = 0;
            eventlog::Log.record(eventlog::OTA_START, ArduinoOTA.getCommand() == U_FLASH ? 0 : 1);
            LOG_I(tag, "update started (%s)", ArduinoOTA.getCommand() == U_FLASH ? "sketch" : "filesystem");
        })
        .onEnd([]() {
            state = OTA_STATE_DONE;
            progress = 100;
            eventlog::Log.record(eventlog::OTA_END, 0);
            LOG_I(tag, "update finished");
        })
        .onProgress([](unsigned int progressDone, unsigned int total) {
            if (total == 0) {
                return;
            }

            progress = static_cast<uint8_t>(progressDone / (total / 100));
        })
        .onError([](ota_error_t error) {
            state = OTA_STATE_FAILED;
            lastError = static_cast<int>(error);
            eventlog::Log.record(eventlog::OTA_ERROR, lastError);
            LOG_E(tag, "update failed, error %d", lastError);
        });

    ArduinoOTA.begin();
}

void OTALoop() {
    ArduinoOTA.handle();
}

OtaState OtaGetState(void) {
    return state;
}

int OtaGetLastError(void) {
    return lastError;
}

uint8_t OtaGetProgress(void) {
    return progress;
}

const char *OtaStateName(OtaState value) {
    switch (value) {
        case OTA_STATE_IDLE:
            return "IDLE";
        case OTA_STATE_RUNNING:
            return "RUNNING";
        case OTA_STATE_DONE:
            return "DONE";
        case OTA_STATE_FAILED:
            return "FAILED";
        default:
            return "UNKNOWN";
    }
}
