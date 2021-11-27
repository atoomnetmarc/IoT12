/*

Copyright 2021 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#include "AdcInternal.h"
#include "AdcExternal.h"
#include "AnalogComparator.h"
#include "Sensor.h"

SensorClass::SensorClass()
{
}

void SensorClass::init(void)
{
    ADCInternal.init();
    ADCExternal.init();
    AnalogComparator.init();
}

void SensorClass::loop(void)
{
    ADCInternal.loop();
    ADCExternal.loop();
    AnalogComparator.loop();
}


SensorClass Sensor;