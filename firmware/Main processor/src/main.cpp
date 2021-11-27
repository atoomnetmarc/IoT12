/*

Copyright 2021 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#include <esp32-hal-cpu.h>

#include <Wire.h>

#include <IoAbstraction.h>
#include <TaskManagerIO.h>

#include "config.h"
#include "display.h"
#include "HeatController.h"
#include "main.h"
#include "webserverc.h"
#include "wific.h"

volatile int encoderValue = 30;
void onEncoderChange(int newValue)
{
    RequestDrawDisplay();

    if (newValue > 0 && encoderValue < 600)
    {
        encoderValue += 5;
    }

    if (newValue < 0 && encoderValue > -10)
    {
        encoderValue -= 5;
    }

    /*
    if (newValue < 0)
        Serial.println("down");
    else
        Serial.println("up");
    */

    float celsius = encoderValue;

    HeatController.setTargetTemperature(celsius + 273.15f);
}

void onSpinwheelClicked(uint8_t pin, bool heldDown)
{
    Serial.print("Button pressed ");
    Serial.println(heldDown ? "Held" : "Pressed");

    /*
  u8g2.beginSimple();
  u8g2.setPowerSave(0);
  */

    if (heldDown == false)
    {
        HEATER_STATE state = HeatController.getHeaterState();
        if (state == HEATER_STATE::OFF)
            HeatController.setHeaterState(HEATER_STATE::ON);
        else
            HeatController.setHeaterState(HEATER_STATE::OFF);
    }

    /*
    FirmwareVersion_t fv = HeatController.getFirmwareVersion();
    Serial.println(String("Version heatcontroller: ") + String(fv.x) + String(".") + String(fv.y) + String(".") + String(fv.z));

    SerialNumber_t sn = HeatController.getSerialNumber();
    Serial.println(String("Serial number heatcontroller: ") + String(sn.word0, HEX) + String(" ") + String(sn.word1, HEX) + String(" ") + String(sn.word2, HEX) + String(" ") + String(sn.word3, HEX) );

    uint16_t const1 = HeatController.getConst1();
    Serial.println(String("Const1 heatcontroller: 0x") + String(const1, HEX) );

    float const2 = HeatController.getConst2();
    Serial.println(String("Const2 heatcontroller: ") + String(const2, 6));
    */
}

void setup()
{
    Wire.begin(SDA, SCL, 400000U);

    Serial.begin(115200);
    Serial.println(F("Booting"));

    setCpuFrequencyMhz(80);
    Serial.println(String(F("CPU freq: ")) + String(ESP.getCpuFreqMHz()) + String(F("MHz")));
    Serial.println(String(F("Flash size: ")) + String(ESP.getFlashChipSize() / 1024 / 1024) + String(F("MiB")));

    DisplaySetup();

    pinMode(PIN_ROT_A, PULLUP);
    pinMode(PIN_ROT_B, PULLUP);
    pinMode(PIN_ROT_SW, PULLUP);
    switches.initialiseInterrupt(ioUsingArduino(), true);
    switches.addSwitch(PIN_ROT_SW, onSpinwheelClicked);
    setupRotaryEncoderWithInterrupt(PIN_ROT_A, PIN_ROT_B, onEncoderChange);
    switches.changeEncoderPrecision(0, 0);

    WificSetup();

    pinMode(PIN_LCD_LED, OUTPUT);
    digitalWrite(PIN_LCD_LED, 1);
    HeatController.init();

    taskManager.scheduleFixedRate(
        200, [] {
            RequestDrawDisplay();
        },
        TIME_MILLIS);
}

void CriticalLoop()
{
    HeatController.loop();
    taskManager.yieldForMicros(0);
    yield();
}

void loop()
{
    CriticalLoop();
    taskManager.runLoop();

    WiFicLoop();
    WebservercLoop();
    DisplayLoop();
}
