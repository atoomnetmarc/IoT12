/*

Copyright 2021-2025 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#include "Sensor.h"
#include "AdcExternal.h"
#include "AdcInternal.h"
#include "AnalogComparator.h"

SensorClass::SensorClass() {
}

void SensorClass::init(void) {
    ADCInternal.init();
    ADCExternal.init();
    AnalogComparator.init();
}

void SensorClass::loop(void) {
    ADCInternal.loop();
    ADCExternal.loop();
    AnalogComparator.loop();
}

SensorClass Sensor;