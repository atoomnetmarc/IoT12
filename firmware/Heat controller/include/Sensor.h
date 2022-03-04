/*

Copyright 2021-2022 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#ifndef Sensor_H
#define Sensor_H

class SensorClass
{
public:
    SensorClass();
    void init(void);
    void loop(void);

private:
};

extern SensorClass Sensor;

#endif