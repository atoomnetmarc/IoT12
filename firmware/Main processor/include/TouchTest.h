/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <stdint.h>

/*
 * Touch test screen.
 *
 * A modal mode on top of the Touch screen. It shows a crosshair and the
 * mapped and raw coordinates of the last touch, and returns to the carousel
 * through the EXIT button or after a timeout without a touch.
 */

namespace touch_test {
    // Initialize the state machine.
    void setup(void);

    // Sample the panel and run the test screen.
    void loop(void);

    // True while the test screen owns the panel.
    bool active(void);

    // Open the test screen. Returns false when it is already open.
    bool start(void);

    // Close the test screen and return to the carousel.
    void stop(void);
}
