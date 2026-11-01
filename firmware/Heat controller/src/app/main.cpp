/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include <Arduino.h>
#include <Wire.h>

#include "AdcCommands.h"
#include "EventLog.h"
#include "Heater.h"
#include "HeaterCommands.h"
#include "Sensor.h"
#include "SerialCli.h"
#include "Setting.h"
#include "SettingCommands.h"
#include "SystemCommands.h"
#include "WireMinion.h"
#include "main.h"

namespace {
    /*
     * Returns immediately when no host is attached, so a missing terminal can
     * never stall the heater loop.
     */
    void serialOutput(const char *data, size_t len) {
        if (!SerialUSB) {
            return;
        }

        SerialUSB.write(reinterpret_cast<const uint8_t *>(data), len);
    }

    cli::SerialCli serialCli(serialOutput);
}

void HardFault_Handler(void) {
#if defined DEBUG
    __BKPT(3);
#endif
    NVIC_SystemReset();
    for (;;) {
    }
}

void setup() {
    SerialUSB.begin(115200);

    eventlog::Log.setTimeSource(millis);
    eventlog::Log.setSink(serialOutput);
    eventlog::Log.record(eventlog::BOOT, 0);
    eventlog::Log.record(eventlog::RESET_CAUSE, PM->RCAUSE.reg);

    system_commands::registerCommands(serialCli);
    heater_commands::registerCommands(serialCli);
    adc_commands::registerCommands(serialCli);
    setting_commands::registerCommands(serialCli);

    Wire.begin();
    Wire.setClock(100000);
    WireMinion.init();

    Setting.init();

    Sensor.init();

    Heater.init();
}

void loop() {
    Heater.loop();
    Sensor.loop();
    Setting.loop();
    WireMinion.loop();

    while (SerialUSB.available() > 0) {
        serialCli.feed(static_cast<char>(SerialUSB.read()));
    }

    if (system_commands::rebootPending()) {
        NVIC_SystemReset();
    }
}
