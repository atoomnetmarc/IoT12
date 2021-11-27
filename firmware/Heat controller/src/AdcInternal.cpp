/*

Copyright 2021 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#include "AdcInternal.h"

ADCInternalClass::ADCInternalClass()
{
}

void ADCInternalClass::init(void)
{
    ADC->CTRLA.bit.ENABLE = 0;
    while (ADC->STATUS.bit.SYNCBUSY == 1)
    {
    }

    ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV64 | ADC_CTRLB_RESSEL_12BIT;
    ADC->AVGCTRL.reg = ADC_AVGCTRL_SAMPLENUM_1 | ADC_AVGCTRL_ADJRES(0x00ul);
    ADC->SAMPCTRL.reg = 0x00;
    ADC->CTRLA.bit.ENABLE = 1;

    while (ADC->STATUS.bit.SYNCBUSY == 1)
    {
    }

    analogReadResolution(12);
    analogReference(AR_INTERNAL2V23);
}

void ADCInternalClass::requestDump(void)
{
    dumpPending = true;
}

float ADCInternalClass::readChannel(pin_size_t pinNumber, uint8_t oversampleExtraBits)
{
    uint16_t loops = 1;
    if (oversampleExtraBits > 0)
        loops = pow(4, oversampleExtraBits);

    uint32_t val = 0;

    for (uint16_t loop = 0; loop < loops; loop++)
        val += analogRead(pinNumber);

    val >>= oversampleExtraBits;

    uint32_t divisor = pow(2, ADC_RESOLUTION + oversampleExtraBits) - 1;

    float voltage = val * 2.23 / divisor;

    return voltage;
}

void ADCInternalClass::loop(void)
{
        if (dumpPending)
    {
        dumpPending = false;

        float voltage;

        SerialUSB.print("NTC: ");
        voltage = readChannel(PIN_INTERNAL_ADC_NTC);
        SerialUSB.print(voltage, 5);
        SerialUSB.print("V");

        SerialUSB.print(", TEMP: ");
        voltage = readChannel(PIN_INTERNAL_ADC_TEMP);
        SerialUSB.print(voltage, 5);
        SerialUSB.print("V");

        SerialUSB.print(", VIN: ");
        voltage = readChannel(PIN_INTERNAL_ADC_VIN);
        SerialUSB.print(voltage, 5);
        SerialUSB.print("V");

        SerialUSB.print(", IRON_CURRENT: ");
        voltage = readChannel(PIN_INTERNAL_ADC_IRON_CURRENT);
        SerialUSB.print(voltage, 5);
        SerialUSB.print("V");

        SerialUSB.print(", AIN_COMP: ");
        voltage = readChannel(PIN_INTERNAL_ADC_AIN_COMP);
        SerialUSB.print(voltage, 5);
        SerialUSB.print("V");

        SerialUSB.println();
    }
}

ADCInternalClass ADCInternal;