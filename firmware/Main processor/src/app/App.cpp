/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include <Arduino.h>
#include <Wire.h>
#include <esp32-hal-cpu.h>

#include "App.h"
#include "DisplayCommands.h"
#include "EventLog.h"
#include "HeatCommands.h"
#include "HeatController.h"
#include "Log.h"
#include "LogCommands.h"
#include "OtaCommands.h"
#include "SerialCli.h"
#include "SystemCommands.h"
#include "Ui.h"
#include "UiCommands.h"
#include "WebCommands.h"
#include "WifiCommands.h"
#include "config.h"
#include "display.h"
#include "ota.h"
#include "webserverc.h"
#include "wific.h"

static const char *tag = "App";

namespace {
    void serialOutput(const char *data, size_t len) {
        Serial.write(reinterpret_cast<const uint8_t *>(data), len);
    }

    cli::SerialCli serialCli(serialOutput);
}

void AppSetup(void) {
    Wire.begin(SDA, SCL, 100000U);

    Serial.begin(115200);

    logger::setSink(serialOutput);
    logger::setTimeSource(millis);
    logger::setLevel(logger::INFO);

    setCpuFrequencyMhz(80);

    eventlog::Log.setTimeSource(millis);
    eventlog::Log.setSink(serialOutput);
    eventlog::Log.record(eventlog::BOOT, 0);
    eventlog::Log.record(eventlog::RESET_CAUSE, static_cast<int32_t>(esp_reset_reason()));

    LOG_I(tag, "boot: version %s, reset reason %d", PRODUCT_VERSION, static_cast<int>(esp_reset_reason()));

    system_commands::registerCommands(serialCli);
    log_commands::registerCommands(serialCli);
    heat_commands::registerCommands(serialCli);
    wifi_commands::registerCommands(serialCli);
    web_commands::registerCommands(serialCli);
    ota_commands::registerCommands(serialCli);
    display_commands::registerCommands(serialCli);
    ui_commands::registerCommands(serialCli);

    DisplaySetup();
    UiSetup();

    WificSetup();

    if (HeatController.init() == false) {
        eventlog::Log.record(eventlog::FAULT, 1);
        LOG_E(tag, "heat controller not reachable on the minion bus");
    } else {
        LOG_I(tag, "heat controller %s, compatible %s", HeatController.getFirmwareVersion(), HeatController.isCompatible() ? "yes" : "no");
    }
}

void AppLoop(void) {
    HeatController.loop();
    UiLoop();

    WiFicLoop();
    WebservercLoop();
    DisplayLoop();

    while (Serial.available() > 0) {
        serialCli.feed(static_cast<char>(Serial.read()));
    }

    if (system_commands::rebootPending()) {
        ESP.restart();
    }
}
