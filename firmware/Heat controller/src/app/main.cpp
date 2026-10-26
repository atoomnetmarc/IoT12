/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include <Arduino.h>
#include <Wire.h>

#include "Heater.h"
#include "Sensor.h"
#include "Setting.h"
#include "WireMinion.h"
#include "main.h"

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
}
