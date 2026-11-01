/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <stdint.h>

// Initialize the panel, touch controller and backlight.
void DisplaySetup(void);

// Draw the home panel when a displayed value changes.
void DisplayLoop(void);

// Name of the display driver in use.
const char *DisplayDriverName(void);

// Panel width in pixels for the active rotation.
uint16_t DisplayWidth(void);

// Panel height in pixels for the active rotation.
uint16_t DisplayHeight(void);

// Active rotation value 0-3.
uint8_t DisplayRotation(void);

// Backlight level 0-255.
uint8_t DisplayBacklight(void);

// Set the backlight level 0-255.
void DisplaySetBacklight(uint8_t level);

// Switch the backlight on or off, keeping the configured level.
void DisplaySetPower(bool on);

// Draw a color bar test pattern once.
void DisplayTestPattern(void);

// Milliseconds of the last completed home panel draw.
uint32_t DisplayLastDrawMs(void);

// Read the touch panel; returns true when pressed and fills raw and mapped coordinates.
bool TouchRead(uint16_t *x, uint16_t *y, uint16_t *rawX, uint16_t *rawY);

// Level of the touch IRQ line, which is active low.
bool TouchIrqLevel(void);
