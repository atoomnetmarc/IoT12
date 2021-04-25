/*

Copyright 2021 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#include <Arduino.h>

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

void ADCInternalClass::loop(void)
{
}

ADCInternalClass ADCInternal;