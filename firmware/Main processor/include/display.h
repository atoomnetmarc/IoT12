/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

// Screens of the touchscreen carousel, in display order.
enum DisplayScreen : uint8_t {
    DISPLAY_SCREEN_HOME = 0,
    DISPLAY_SCREEN_NETWORK,
    DISPLAY_SCREEN_INFO,
};

// Initialize the panel, touch controller and backlight.
void DisplaySetup(void);

// Draw the active screen when a displayed value changes.
void DisplayLoop(void);

// Mark the displayed values dirty so the next DisplayLoop() re-reads them and
// redraws only the fields that changed. This never wipes the whole panel.
void DisplayRequestDraw(void);

// Move the carousel one screen forward or back; the arrows wrap around.
void DisplayScreenNext(void);
void DisplayScreenPrev(void);

// Select a screen directly.
void DisplayScreenSet(DisplayScreen screen);

// Active screen, its name and the number of screens.
DisplayScreen DisplayScreenGet(void);
const char *DisplayScreenName(DisplayScreen screen);
size_t DisplayScreenCount(void);

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

// Draw the panel edge and a rectangle inset by margin pixels, to measure the
// visible area under the bezel. Stays on screen until DisplayBoxStop().
void DisplayDrawBox(uint16_t margin);
void DisplayBoxStop(void);
bool DisplayBoxActive(void);

// Milliseconds of the last completed draw.
uint32_t DisplayLastDrawMs(void);

// Screenshot support. Begin() takes the panel for the whole capture, Read()
// copies one strip of RGB565 pixels, End() releases the panel. The caller must
// pair Begin() with End().
bool DisplayScreenshotBegin(uint32_t timeoutMs);
void DisplayScreenshotRead(uint16_t y, uint16_t h, uint16_t *dst);
void DisplayScreenshotEnd(void);

// Read the touch panel; returns true when pressed and fills raw and mapped coordinates.
bool TouchRead(uint16_t *x, uint16_t *y, uint16_t *rawX, uint16_t *rawY);

// Level of the touch IRQ line, which is active low.
bool TouchIrqLevel(void);
