/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "display.h"

/*
 * The 5110 LCD was removed in V2.2 and the U8g2 driver went with it. The
 * ILI9341 panel on LovyanGFX lands in M3; until then the display is a no-op so
 * the rest of the firmware can be built and tested.
 */

void RequestDrawDisplay(void) {
}

void DisplaySetup(void) {
}

void DisplayLoop(void) {
}

const char *DisplayDriverName(void) {
    return "none (M3)";
}
