/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include "Temperature.h"

/*
 * The temperature unit that the web UI uses. The id is stored in NVS; an
 * unknown or missing id falls back to Celsius.
 */
namespace temperature_unit {
    void setup(void);
    const temperature::Unit *active(void);
    const char *id(void);
    bool set(const char *id);
}
