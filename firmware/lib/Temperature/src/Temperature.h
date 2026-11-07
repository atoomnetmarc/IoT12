/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <cstddef>
#include <cstdint>

/*
 * Temperature scales. Kelvin is the internal unit; every scale converts to and
 * from Kelvin. The table order is the display order, with the common scales
 * first.
 */
namespace temperature {
    struct Unit {
        const char *id;
        const char *name;
        const char *symbol;
        float (*toKelvin)(float value);
        float (*fromKelvin)(float kelvin);
        float step;
        int8_t direction;
        uint8_t decimals;
        bool common;
    };

    const Unit *units(void);
    size_t unitCount(void);
    const Unit *find(const char *id);
    const Unit *celsius(void);
}
