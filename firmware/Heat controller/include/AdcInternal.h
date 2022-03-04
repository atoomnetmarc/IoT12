/*

Copyright 2021-2022 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#ifndef AdcInternal_H
#define AdcInternal_H

#include <Arduino.h>
#include <Wire.h>

#include "config.h"

class ADCInternalClass
{
public:
    ADCInternalClass();
    void init(void);
    void loop(void);
    float readChannel(pin_size_t pinNumber, uint8_t oversampleExtraBits = 2);
    void requestDump(void);
private:
    volatile bool dumpPending = false;
};

extern ADCInternalClass ADCInternal;

#endif