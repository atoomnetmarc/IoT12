/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

/*
 * Touch panel calibration.
 *
 * The calibration runs as a non-blocking state machine in the main loop. It
 * collects four corner taps, extrapolates them to the native panel corners,
 * applies the result with setTouchCalibrate(), and validates the mapping with
 * four small buttons before it stores the eight raw corner values in NVS.
 */
namespace touch_calibration {
    struct Rect {
        int16_t x;
        int16_t y;
        int16_t w;
        int16_t h;
    };

    // Corner targets in landscape rotation 1, inside the 16 px bezel.
    constexpr Rect TARGET_RECTS[4] = {
        {32, 32, 32, 32},
        {272, 32, 32, 32},
        {32, 192, 32, 32},
        {272, 192, 32, 32},
    };

    // Validation buttons in landscape rotation 1, inside the 16 px bezel.
    constexpr Rect VALIDATION_RECTS[4] = {
        {16, 16, 32, 32},
        {272, 16, 32, 32},
        {16, 192, 32, 32},
        {272, 192, 32, 32},
    };

    // Load the stored calibration and apply it; the panel config is the fallback.
    void setup(void);

    // Run the calibration state machine; returns immediately.
    void loop(void);

    // True while the calibration mode owns the panel.
    bool active(void);

    // Start the calibration; returns false when one is already active.
    bool start(void);

    // Stop the calibration and restore the previous mapping.
    void abort(void);

    // Clear the stored calibration and restore the panel config fallback.
    void reset(void);

    // True when a calibration is stored in NVS.
    bool stored(void);

    // Number of stored values and the value at an index.
    size_t valueCount(void);
    uint16_t valueAt(size_t index);

    // Name of the active step, for the CLI.
    const char *stepName(void);
}
