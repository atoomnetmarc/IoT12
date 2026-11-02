/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "AdcInternal.h"

// Configures the internal ADC for 12-bit conversions against the 2.23 V reference.
void ADCInternalClass::init(void) {
    ADC->CTRLA.bit.ENABLE = 0;
    while (ADC->STATUS.bit.SYNCBUSY == 1) {
    }

    ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV64 | ADC_CTRLB_RESSEL_12BIT;
    ADC->AVGCTRL.reg = ADC_AVGCTRL_SAMPLENUM_1 | ADC_AVGCTRL_ADJRES(0x00ul);
    ADC->SAMPCTRL.reg = 0x00;
    ADC->CTRLA.bit.ENABLE = 1;

    while (ADC->STATUS.bit.SYNCBUSY == 1) {
    }

    analogReadResolution(12);
    analogReference(AR_INTERNAL2V23);
}

// Reads one pin with 4^oversampleExtraBits samples and returns the voltage.
float ADCInternalClass::readChannel(pin_size_t pinNumber, uint8_t oversampleExtraBits) {
    uint16_t loops = 1u << (2u * oversampleExtraBits);
    uint32_t val = 0;

    for (uint16_t loop = 0; loop < loops; loop++) {
        val += analogRead(pinNumber);
    }

    val >>= oversampleExtraBits;

    uint32_t divisor = (1u << (ADC_RESOLUTION + oversampleExtraBits)) - 1u;

    return val * 2.23f / divisor;
}

ADCInternalClass ADCInternal;
