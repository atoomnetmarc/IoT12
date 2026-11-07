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
    // The high-impedance temperature sensor needs the maximum sample time to settle.
    ADC->SAMPCTRL.reg = 0x3F;
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

// Reads one pin against the 3.3 V supply reference and restores the 2.23 V reference.
float ADCInternalClass::readChannelVdd(pin_size_t pinNumber, uint8_t oversampleExtraBits) {
    analogReference(AR_DEFAULT);
    delayMicroseconds(10);

    uint16_t loops = 1u << (2u * oversampleExtraBits);
    uint32_t val = 0;

    for (uint16_t loop = 0; loop < loops; loop++) {
        val += analogRead(pinNumber);
    }

    val >>= oversampleExtraBits;

    uint32_t divisor = (1u << (ADC_RESOLUTION + oversampleExtraBits)) - 1u;
    float result = val * 3.3f / divisor;

    analogReference(AR_INTERNAL2V23);
    delayMicroseconds(10);

    return result;
}

// Reads one internal ADC channel and returns the voltage against the given reference.
float ADCInternalClass::readInternalChannel(uint8_t muxpos, float referenceVoltage, uint8_t oversampleExtraBits) {
    uint16_t loops = 1u << (2u * oversampleExtraBits);
    uint32_t val = 0;

    for (uint16_t loop = 0; loop < loops; loop++) {
        while (ADC->STATUS.bit.SYNCBUSY == 1) {
        }
        ADC->INPUTCTRL.bit.MUXPOS = muxpos;

        while (ADC->STATUS.bit.SYNCBUSY == 1) {
        }
        ADC->CTRLA.bit.ENABLE = 0x01;

        while (ADC->STATUS.bit.SYNCBUSY == 1) {
        }
        ADC->SWTRIG.bit.START = 1;

        while (ADC->INTFLAG.bit.RESRDY == 0) {
        }
        ADC->INTFLAG.reg = ADC_INTFLAG_RESRDY;

        // The first conversion after a reference change is discarded.
        while (ADC->STATUS.bit.SYNCBUSY == 1) {
        }
        ADC->SWTRIG.bit.START = 1;

        while (ADC->INTFLAG.bit.RESRDY == 0) {
        }
        val += ADC->RESULT.reg;

        while (ADC->STATUS.bit.SYNCBUSY == 1) {
        }
        ADC->CTRLA.bit.ENABLE = 0x00;
    }

    val >>= oversampleExtraBits;

    uint32_t divisor = (1u << (ADC_RESOLUTION + oversampleExtraBits)) - 1u;

    return val * referenceVoltage / divisor;
}

// Reads the SAMD21 die temperature from the internal sensor.
float ADCInternalClass::readDieTemperature(uint8_t oversampleExtraBits) {
    analogReference(AR_INTERNAL2V23);
    delayMicroseconds(10);

    float voltage = readInternalChannel(ADC_INPUTCTRL_MUXPOS_TEMP_Val, 2.23f, oversampleExtraBits);

    return 25.0f + (voltage - 0.7f) / -0.00175f;
}

// Reads the SAMD21 I/O supply through the 1/4 scaled internal channel.
float ADCInternalClass::readVdd(uint8_t oversampleExtraBits) {
    analogReference(AR_INTERNAL1V0);
    delayMicroseconds(10);

    float voltage = readInternalChannel(ADC_INPUTCTRL_MUXPOS_SCALEDIOVCC_Val, 1.0f, oversampleExtraBits);

    analogReference(AR_INTERNAL2V23);
    delayMicroseconds(10);

    return voltage * 4.0f;
}

ADCInternalClass ADCInternal;
