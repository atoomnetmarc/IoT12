/*

Copyright 2021 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#include <ADS1115_WE.h>

#include "AdcExternal.h"
#include "config.h"

volatile bool convReady = false;

void convReadyAlert()
{
    convReady = true;
}

ADCExternalClass::ADCExternalClass()
{
}

ADS1115_WE adc(I2C_ADS1115_ADDRESS);

void ADCExternalClass::init(void)
{
    adc.init();
    adc.setVoltageRange_mV(ADS1115_RANGE_2048);
    adc.setCompareChannels(ADS1115_COMP_0_GND);
    adc.setAlertPinMode(ADS1115_ASSERT_AFTER_1);
    //adc.setConvRate(ADS1115_128_SPS);
    adc.setConvRate(ADS1115_860_SPS);
    adc.setAlertPinToConversionReady();
    pinMode(PIN_ADC_READY, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_ADC_READY), convReadyAlert, FALLING);
}

float readChannel(ADS1115_MUX channel)
{
    float voltage = 0.0;
    adc.setCompareChannels(channel);
    adc.startSingleMeasurement();
    while (convReady == false)
    {
    }
    convReady = false;
    voltage = adc.getResult_V(); // alternative: getResult_mV for Millivolt
    return voltage;
}

void ADCExternalClass::loop(void)
{
    /*
    static unsigned long timer2 = 0;
    if (timer2 + 100 < millis())
    {
        timer2 = millis();

        float voltage = 0.0;

        SerialUSB.print("0: ");
        voltage = readChannel(ADS1115_COMP_0_GND);
        SerialUSB.print(voltage, 5);

        SerialUSB.print(",   1: ");
        voltage = readChannel(ADS1115_COMP_1_GND);
        SerialUSB.print(voltage, 5);

        SerialUSB.print(",   2: ");
        voltage = readChannel(ADS1115_COMP_2_GND);
        SerialUSB.print(voltage, 5);

        SerialUSB.print(",   3: ");
        voltage = readChannel(ADS1115_COMP_3_GND);
        SerialUSB.println(voltage, 5);
    }
    */
}

ADCExternalClass ADCExternal;
