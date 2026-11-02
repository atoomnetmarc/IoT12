/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <Arduino.h>

/*
 * Read-only diagnostic for the SAMD21 internal ADC. The ADS1115 is the
 * measurement source of truth; this class reads the same analog nets so the
 * two paths can be compared. It never feeds the heater control loop.
 */
class ADCInternalClass {
public:
    void init(void);
    float readChannel(pin_size_t pinNumber, uint8_t oversampleExtraBits = 2);
};

extern ADCInternalClass ADCInternal;
