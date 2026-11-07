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

// Runtime properties of the panel, its bus, the touch controller and the backlight.
struct DisplayInfo {
    const char *driver;
    uint16_t width;
    uint16_t height;
    uint8_t rotation;
    uint8_t backlight;
    uint32_t lastDrawMs;
    uint8_t colorDepth;

    int16_t pinCs;
    int16_t pinRst;
    int16_t pinBusy;
    uint16_t memoryWidth;
    uint16_t memoryHeight;
    uint16_t panelWidth;
    uint16_t panelHeight;
    uint16_t offsetX;
    uint16_t offsetY;
    uint8_t offsetRotation;
    uint8_t dummyReadPixel;
    uint8_t dummyReadBits;
    uint16_t endReadDelayUs;
    bool readable;
    bool invert;
    bool rgbOrder;
    bool dlen16bit;
    bool busShared;

    uint32_t freqWrite;
    uint32_t freqRead;
    int16_t pinSclk;
    int16_t pinMiso;
    int16_t pinMosi;
    int16_t pinDc;
    int16_t pinIo0;
    int16_t pinIo1;
    int16_t pinIo2;
    int16_t pinIo3;
    uint8_t spiMode;
    bool spi3wire;
    bool useLock;
    uint8_t dmaChannel;
    int spiHost;

    uint32_t touchFreq;
    uint16_t touchXMin;
    uint16_t touchXMax;
    uint16_t touchYMin;
    uint16_t touchYMax;
    bool touchBusShared;
    int16_t touchPinInt;
    int16_t touchPinRst;
    uint8_t touchOffsetRotation;
    int touchSpiHost;
    int16_t touchPinSclk;
    int16_t touchPinMosi;
    int16_t touchPinMiso;
    int16_t touchPinCs;

    uint32_t lightFreq;
    int16_t lightPinBl;
    uint8_t lightOffset;
    uint8_t lightPwmChannel;
    bool lightInvert;

    uint32_t id1;
    uint32_t id4;
    uint32_t id9;
    uint32_t idc;
};

// Fill the runtime properties of the panel, its bus, the touch controller and
// the backlight. The controller IDs are read over SPI.
void DisplayGetInfo(DisplayInfo *info);

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
