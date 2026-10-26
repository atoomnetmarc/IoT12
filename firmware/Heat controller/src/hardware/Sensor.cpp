/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "Sensor.h"
#include "AdcExternal.h"
#include "AnalogComparator.h"

SensorClass::SensorClass() {
}

void SensorClass::init(void) {
    ADCExternal.init();
    AnalogComparator.init();
}

void SensorClass::loop(void) {
    ADCExternal.loop();
    AnalogComparator.loop();
}

SensorClass Sensor;