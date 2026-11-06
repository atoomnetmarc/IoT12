/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "TouchTest.h"

#include <Arduino.h>

#include "Log.h"
#include "Ui.h"
#include "display.h"

namespace {
    constexpr const char *TAG = "TouchTest";
    constexpr uint32_t SAMPLE_MS = 20;
    constexpr uint32_t DEBOUNCE_MS = 30;
    constexpr uint32_t DRAW_MS = 100;
    constexpr uint32_t TIMEOUT_MS = 30000;
    constexpr uint16_t MOVE_PX = 8;

    bool running = false;
    bool rawDown = false;
    bool stableDown = false;
    bool havePoint = false;
    bool exitPressed = false;
    uint32_t lastSampleMs = 0;
    uint32_t rawChangeMs = 0;
    uint32_t lastDrawMs = 0;
    uint32_t lastTouchMs = 0;
    uint16_t pointX = 0;
    uint16_t pointY = 0;
    uint16_t pointRawX = 0;
    uint16_t pointRawY = 0;

    // True when a point is inside a rectangle.
    bool inside(const UiRect &r, uint16_t x, uint16_t y) {
        return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
    }

    // Squared distance between two points, so no square root is needed.
    uint32_t distanceSquared(uint16_t ax, uint16_t ay, uint16_t bx, uint16_t by) {
        int32_t dx = static_cast<int32_t>(ax) - static_cast<int32_t>(bx);
        int32_t dy = static_cast<int32_t>(ay) - static_cast<int32_t>(by);

        return static_cast<uint32_t>(dx * dx + dy * dy);
    }

    // Repaint the test screen with the current point.
    void redraw(void) {
        DisplayTouchTestDraw(havePoint, pointX, pointY, pointRawX, pointRawY, exitPressed);
        lastDrawMs = millis();
    }
}

void touch_test::setup(void) {
    running = false;
    rawDown = false;
    stableDown = false;
    havePoint = false;
    exitPressed = false;
}

bool touch_test::active(void) {
    return running;
}

bool touch_test::start(void) {
    if (running) {
        return false;
    }

    running = true;
    havePoint = false;
    exitPressed = false;
    lastSampleMs = millis();
    rawChangeMs = lastSampleMs;
    lastDrawMs = 0;
    lastTouchMs = lastSampleMs;

    // The press that opened the screen is still down. Swallow it, so the
    // EXIT button under the finger does not close the screen immediately.
    bool down = false;
    if (!TouchIrqLevel()) {
        down = TouchRead(nullptr, nullptr, nullptr, nullptr);
    }
    rawDown = down;
    stableDown = down;

    redraw();
    LOG_I(TAG, "test screen started");

    return true;
}

void touch_test::stop(void) {
    if (!running) {
        return;
    }

    running = false;
    exitPressed = false;
    DisplayTouchTestEnd();
    LOG_I(TAG, "test screen stopped");
}

void touch_test::loop(void) {
    if (!running) {
        return;
    }

    uint32_t now = millis();

    if (now - lastTouchMs >= TIMEOUT_MS) {
        LOG_I(TAG, "test screen timeout");
        stop();
        return;
    }

    if (now - lastSampleMs < SAMPLE_MS) {
        return;
    }
    lastSampleMs = now;

    bool down = false;
    uint16_t x = 0;
    uint16_t y = 0;
    uint16_t rawX = 0;
    uint16_t rawY = 0;

    if (!TouchIrqLevel()) {
        down = TouchRead(&x, &y, &rawX, &rawY);
    }

    if (down != rawDown) {
        rawDown = down;
        rawChangeMs = now;
    }

    if (rawDown != stableDown && now - rawChangeMs >= DEBOUNCE_MS) {
        stableDown = rawDown;

        if (stableDown) {
            lastTouchMs = now;

            if (inside(UI_EXIT_RECT, x, y)) {
                exitPressed = true;
                redraw();
                return;
            }

            exitPressed = false;
            pointX = x;
            pointY = y;
            pointRawX = rawX;
            pointRawY = rawY;
            havePoint = true;
            redraw();
        } else if (exitPressed) {
            // Act on release, so the press that closes the screen is not
            // seen as a new TEST press by the UI.
            exitPressed = false;
            stop();
            return;
        }
    }

    // Only follow the finger while the panel still reports a press. After a
    // release the debounce keeps stableDown true for a few samples, and the
    // coordinates of those samples are zero, which would jump the crosshair
    // to the top left corner.
    if (stableDown && rawDown && !exitPressed) {
        lastTouchMs = now;

        if (now - lastDrawMs >= DRAW_MS && distanceSquared(x, y, pointX, pointY) >= MOVE_PX * MOVE_PX) {
            pointX = x;
            pointY = y;
            pointRawX = rawX;
            pointRawY = rawY;
            redraw();
        }
    }
}
