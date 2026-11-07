/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <stdint.h>

// State of the connected handle, in priority order.
enum class HandleState : uint8_t {
    OK,
    LOSS,
    NO_HANDLE,
    NO_TIP,
    SUPPLY_OFF,
};

// Two-line message for the HOME screen. Both pointers are null in the OK state.
struct HandleDiagnosis {
    HandleState state;
    const char *line1;
    const char *line2;
};

// Classify the handle from the heat controller measurements.
HandleDiagnosis HandleDiagnosisClassify(void);

// Short name of the state, for the CLI.
const char *HandleStateName(HandleState state);
