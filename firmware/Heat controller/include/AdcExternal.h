/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <Arduino.h>
#include <Wire.h>

#include "config.h"
#include <ADS1115_WE.h>

class ADCExternalClass {
public:
    ADCExternalClass();
    void init(void);
    float readChannel(uint8_t channel);
    void readChannelRaw(uint8_t channel, int16_t &raw, float &volts);

    static void convReadyAlert(void);

private:
    volatile bool convReady = false;
};

extern ADCExternalClass ADCExternal;
