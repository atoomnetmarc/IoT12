/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "Sensor.h"
#include "AdcExternal.h"
#include "AdcInternal.h"

SensorClass::SensorClass() {
}

void SensorClass::init(void) {
    ADCExternal.init();
    ADCInternal.init();
}

SensorClass Sensor;