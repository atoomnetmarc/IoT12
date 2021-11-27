/*

Copyright 2021 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#include <Arduino.h>
#include <Wire.h>

#include "Heater.h"
#include "main.h"
#include "Sensor.h"
#include "Setting.h"
#include "WireMinion.h"

void HardFault_Handler(void)
{
#if defined DEBUG
    __BKPT(3);
#endif
    NVIC_SystemReset();
    for (;;)
    {
    }
}

void setup()
{
    pinMode(0, OUTPUT);  //TP1
    pinMode(1, OUTPUT);  //TP2
    pinMode(9, OUTPUT);  //TP3
    pinMode(10, OUTPUT); //TP4
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    SerialUSB.begin(115200);

    Wire.begin();
    Wire.setClock(100000);
    WireMinion.init();

    Setting.init();

    I2SGPIO.init();
    Sensor.init();

    Heater.init();
}

void loop()
{
    Heater.loop();
    Sensor.loop();
    Setting.loop();
    WireMinion.loop();
}
