/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#include "display.h"

/*
 * Touch UI for the screen carousel.
 *
 * The geometry lives here because two modules need the same numbers: the
 * display draws the boxes and the UI hit-tests them. Everything is inside
 * the 16 px bezel safe area.
 */

enum UiZone : uint8_t {
    UI_ZONE_NONE = 0,
    UI_ZONE_PREV,
    UI_ZONE_NEXT,
    UI_ZONE_POWER,
    UI_ZONE_PLUS,
    UI_ZONE_MINUS,
    UI_ZONE_AP,
};

struct UiRect {
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
};

constexpr int16_t UI_SAFE_MARGIN = 16;
constexpr UiRect UI_PREV_RECT = {16, 16, 32, 32};
constexpr UiRect UI_NEXT_RECT = {272, 16, 32, 32};
constexpr UiRect UI_POWER_RECT = {16, 164, 96, 60};
constexpr UiRect UI_PLUS_RECT = {112, 164, 96, 60};
constexpr UiRect UI_MINUS_RECT = {208, 164, 96, 60};
constexpr UiRect UI_AP_RECT = {112, 164, 96, 60};

// Initialize the touch state machine.
void UiSetup(void);

// Sample the touch panel and run the press, hold and repeat state machine.
void UiLoop(void);

// Zone under a panel coordinate for the active screen, or UI_ZONE_NONE.
UiZone UiHitTest(uint16_t x, uint16_t y);

// Zone currently held down, or UI_ZONE_NONE.
UiZone UiPressedZone(void);

// Name of a zone, for the CLI.
const char *UiZoneName(UiZone zone);

// Bounds of a zone, for the CLI.
const UiRect &UiZoneRect(UiZone zone);

// Number of zones and the zone at an index, for the CLI.
size_t UiZoneCount(void);
UiZone UiZoneAt(size_t index);

// Setpoint in tenths of a degree Celsius, as the heat controller reports it.
int16_t UiSetpointTenths(void);

// Heater state as the heat controller reports it.
bool UiHeaterOn(void);

// Last touch event, for the CLI.
const char *UiLastEvent(void);

// Clamp and write a setpoint in tenths of a degree Celsius.
void UiSetSetpointTenths(int16_t tenths);

// Run one synthetic tap through the same hit-test and action path.
UiZone UiSyntheticPress(uint16_t x, uint16_t y);
