/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "AdcExternal.h"

namespace {
    // A conversion at 860 SPS takes 1.16 ms; 5 ms leaves room for I2C overhead.
    constexpr uint32_t CONVERSION_TIMEOUT_US = 5000;
}

ADCExternalClass::ADCExternalClass() {
}

ADS1115_WE adc(I2C_ADDRESS_ADS1115);

void ADCExternalClass::init(void) {
    adc.init();
    adc.setVoltageRange_mV(ADS1115_RANGE_4096);
    adc.setCompareChannels(ADS1115_COMP_0_GND);
    adc.setAlertPinMode(ADS1115_ASSERT_AFTER_1);
    adc.setConvRate(ADS1115_860_SPS);
    adc.setAlertPinToConversionReady();
    pinMode(PIN_ADC_READY, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_ADC_READY), &ADCExternalClass::convReadyAlert, FALLING);
}

float ADCExternalClass::readChannel(uint8_t channel) {
    int16_t raw;
    float voltage;

    readChannelRaw(channel, raw, voltage);

    return voltage;
}

// Reads one channel and waits for the conversion-ready edge, bounded by the timeout.
void ADCExternalClass::readChannelRaw(uint8_t channel, int16_t &raw, float &volts) {
    ADS1115_MUX mux = (ADS1115_MUX)(ADS1115_COMP_0_GND + channel * ADS1115_COMP_INC);

    adc.setCompareChannels(mux);
    adc.startSingleMeasurement();

    uint32_t start = micros();
    while (convReady == false && (micros() - start) < CONVERSION_TIMEOUT_US) {
    }
    convReady = false;

    raw = adc.getRawResult();
    volts = adc.getResult_V();
}

void ADCExternalClass::convReadyAlert(void) {
    ADCExternal.convReady = true;
}

ADCExternalClass ADCExternal;
