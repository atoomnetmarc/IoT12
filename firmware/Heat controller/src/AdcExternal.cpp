/*

Copyright 2021 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#include "AdcExternal.h"

ADCExternalClass::ADCExternalClass()
{
}

ADS1115_WE adc(I2C_ADDRESS_ADS1115);

void ADCExternalClass::init(void)
{
    adc.init();
    adc.setVoltageRange_mV(ADS1115_RANGE_4096);
    adc.setCompareChannels(ADS1115_COMP_0_GND);
    adc.setAlertPinMode(ADS1115_ASSERT_AFTER_1);
    adc.setConvRate(ADS1115_860_SPS);
    adc.setAlertPinToConversionReady();
    pinMode(PIN_ADC_READY, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_ADC_READY), &ADCExternalClass::convReadyAlert, FALLING);
}

void ADCExternalClass::requestDump(void)
{
    dumpPending = true;
}

float ADCExternalClass::readChannel(uint8_t channel)
{
    ADS1115_MUX mux = (ADS1115_MUX)(ADS1115_COMP_0_GND + channel * ADS1115_COMP_INC);
    float voltage;
    adc.setCompareChannels(mux);
    adc.startSingleMeasurement();
    while (convReady == false)
    {
    }
    convReady = false;
    voltage = adc.getResult_V();
    return voltage;
}

void ADCExternalClass::loop(void)
{
    if (dumpPending)
    {
        dumpPending = false;

        float voltage;

        SerialUSB.print("NTC: ");
        voltage = readChannel(AIN_EXTERNAL_ADC_NTC);
        SerialUSB.print(voltage, 5);
        SerialUSB.print("V");

        SerialUSB.print(", TEMP: ");
        voltage = readChannel(AIN_EXTERNAL_ADC_TEMP);
        SerialUSB.print(voltage, 5);
        SerialUSB.print("V");

        SerialUSB.print(", VIN: ");
        voltage = readChannel(AIN_EXTERNAL_ADC_VIN);
        SerialUSB.print(voltage, 5);
        SerialUSB.print("V");

        SerialUSB.print(", IRON_CURRENT: ");
        voltage = readChannel(AIN_EXTERNAL_ADC_IRON_CURRENT);
        SerialUSB.print(voltage, 5);
        SerialUSB.print("V");

        SerialUSB.println();
    }
}

void ADCExternalClass::convReadyAlert(void)
{
    ADCExternal.convReady = true;
}

ADCExternalClass ADCExternal;
