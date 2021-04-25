/*

Copyright 2021 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#include <math.h>

#include "lib.h"
#include "Thermocouple.h"

Thermocouple::Thermocouple(void)
{
}

float Thermocouple::GetTemperature(float voltage)
{
    float mv = voltage * 1000;

    float lower_mv = 0;
    float higher_mv = 0;
    uint16_t lower_c = 0;
    uint16_t higher_c = 0;

    if (voltage <= 0)
        return 0;

    for (uint16_t i = 0; i < sizeof(thermocouple_type_c) / sizeof(temp_couple_t); i++)
    {
        if (thermocouple_type_c[i].mv > mv)
        {
            higher_mv = thermocouple_type_c[i].mv;
            higher_c = thermocouple_type_c[i].celsius;
            break;
        }

        lower_mv = thermocouple_type_c[i].mv;
        lower_c = thermocouple_type_c[i].celsius;
    }

    if ((higher_mv < lower_mv) || (higher_mv == 0 && lower_mv == 0))
        return 999999;

    return mapf(mv, lower_mv, higher_mv, lower_c, higher_c);
}
