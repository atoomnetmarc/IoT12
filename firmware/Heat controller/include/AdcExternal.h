/*

Copyright 2021-2022 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#ifndef AdcExternal_H
#define AdcExternal_H

#include <Arduino.h>
#include <Wire.h>

#include <ADS1115_WE.h>
#include "config.h"

class ADCExternalClass
{
public:
    ADCExternalClass();
    void init(void);
    void loop(void);
    float readChannel(uint8_t channel);
    void requestDump(void);

    static void convReadyAlert(void);
private:
    volatile bool dumpPending = false;
    volatile bool convReady = false;
};

extern ADCExternalClass ADCExternal;

#endif