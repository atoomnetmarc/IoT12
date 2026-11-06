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
    DISPLAY_SCREEN_TOUCH,
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

// Apply eight raw corner values to the touch mapping.
void DisplayTouchCalibrationApply(const uint16_t values[8]);

// Fill eight raw corner values from the panel config fallback.
void DisplayTouchCalibrationFallback(uint16_t values[8]);

// Calibration screen. Begin() clears the panel, Target() and Validation() draw
// one step, Message() shows a two-line status, End() returns to the carousel.
// Target() shows white when idle, yellow while pressed and green when the
// minimum hold time has passed.
void DisplayCalibrationBegin(void);
void DisplayCalibrationTarget(uint8_t index, bool pressed, bool ready);
void DisplayCalibrationValidation(uint8_t index, bool pressed, bool hit);
void DisplayCalibrationMessage(const char *line1, const char *line2);
void DisplayCalibrationEnd(void);

// Touch test screen. Draw() repaints the whole screen with the last touch
// point, or without a crosshair when havePoint is false. End() returns to the
// carousel.
void DisplayTouchTestDraw(bool havePoint, uint16_t x, uint16_t y, uint16_t rawX, uint16_t rawY, bool exitPressed);
void DisplayTouchTestEnd(void);
