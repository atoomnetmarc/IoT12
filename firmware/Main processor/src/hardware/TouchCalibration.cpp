/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "TouchCalibration.h"

#include <Arduino.h>
#include <Preferences.h>

#include <cstdio>
#include <cstring>

#include "Log.h"
#include "WebConfigStore.h"
#include "display.h"

namespace {
    constexpr const char *TAG = "TouchCal";
    constexpr const char *NVS_KEY_TOUCH_CAL = "touch_cal";
    constexpr uint32_t STEP_TIMEOUT_MS = 30000;
    constexpr uint32_t TOUCH_SAMPLE_MS = 20;
    constexpr uint32_t DEBOUNCE_MS = 30;
    constexpr uint32_t HOLD_MS = 500;
    constexpr uint32_t RESTART_DELAY_MS = 1000;

    enum Phase : uint8_t {
        PHASE_IDLE = 0,
        PHASE_CORNER,
        PHASE_VALIDATE,
    };

    Phase phase = PHASE_IDLE;
    uint8_t stepIndex = 0;
    uint32_t stepStartMs = 0;
    uint32_t lastSampleMs = 0;
    uint32_t rawChangeMs = 0;
    bool rawDown = false;
    bool stableDown = false;
    bool sampling = false;
    bool holdReady = false;
    bool validationHit = false;
    uint32_t pressStartMs = 0;
    bool restartPending = false;
    uint32_t restartAtMs = 0;
    uint32_t rawSumX = 0;
    uint32_t rawSumY = 0;
    uint16_t sampleCount = 0;
    uint16_t cornerRaw[4][2] = {};
    uint16_t current[8] = {};
    uint16_t previous[8] = {};
    uint16_t candidate[8] = {};
    bool hasStored = false;

    // True when a point is inside a rectangle.
    bool inside(const touch_calibration::Rect &r, uint16_t x, uint16_t y) {
        return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
    }

    // Clamp an extrapolated raw value to the XPT2046 range.
    uint16_t clampRaw(float value) {
        if (value < 0.0f) {
            return 0;
        }
        if (value > 4095.0f) {
            return 4095;
        }
        return static_cast<uint16_t>(value + 0.5f);
    }

    /*
     * Extrapolate the four raw taps to the native panel corners. The crosshair
     * sits at the centre of each target, so the taps are at native (191,48),
     * (191,288), (31,48) and (31,288); the bilinear form is exact for an affine
     * raw-to-native map.
     */
    void computeCandidate(void) {
        auto rawAt = [](int16_t x, int16_t y, uint16_t &rawX, uint16_t &rawY) {
            const float fx = (x - 31.0f) / (191.0f - 31.0f);
            const float fy = (y - 48.0f) / (288.0f - 48.0f);
            const float r00x = cornerRaw[2][0];
            const float r10x = cornerRaw[0][0];
            const float r01x = cornerRaw[3][0];
            const float r11x = cornerRaw[1][0];
            const float r00y = cornerRaw[2][1];
            const float r10y = cornerRaw[0][1];
            const float r01y = cornerRaw[3][1];
            const float r11y = cornerRaw[1][1];
            const float vx = r00x + fx * (r10x - r00x) + fy * (r01x - r00x) + fx * fy * (r11x - r10x - r01x + r00x);
            const float vy = r00y + fx * (r10y - r00y) + fy * (r01y - r00y) + fx * fy * (r11y - r10y - r01y + r00y);

            rawX = clampRaw(vx);
            rawY = clampRaw(vy);
        };

        rawAt(0, 0, candidate[0], candidate[1]);
        rawAt(0, 319, candidate[2], candidate[3]);
        rawAt(239, 0, candidate[4], candidate[5]);
        rawAt(239, 319, candidate[6], candidate[7]);
    }

    // Store the validated values and leave the calibration mode.
    void save(void) {
        Preferences preferences;

        if (preferences.begin(web_config::NVS_NAMESPACE, false)) {
            preferences.putBytes(NVS_KEY_TOUCH_CAL, candidate, sizeof(candidate));
            preferences.end();
        }

        memcpy(current, candidate, sizeof(current));
        hasStored = true;
        DisplayTouchCalibrationApply(current);
        phase = PHASE_IDLE;
        DisplayCalibrationEnd();
        LOG_I(TAG, "calibration saved");
    }

    // Handle a stable press in the active phase.
    void onPress(uint16_t x, uint16_t y) {
        if (phase == PHASE_CORNER) {
            // The current mapping is what is being calibrated, so the reported
            // position cannot be trusted. Accept any press and sample the raw
            // values; the user is told to press the crosshair.
            sampling = true;
            holdReady = false;
            sampleCount = 0;
            rawSumX = 0;
            rawSumY = 0;
            pressStartMs = millis();
            LOG_I(TAG, "corner %u press", static_cast<unsigned>(stepIndex + 1));
            DisplayCalibrationTarget(stepIndex, true, false);
            return;
        }

        if (phase == PHASE_VALIDATE) {
            const touch_calibration::Rect &r = touch_calibration::VALIDATION_RECTS[stepIndex];
            validationHit = inside(r, x, y);
            LOG_I(TAG, "validate %u press %u,%u rect %d,%d,%d,%d hit %s", static_cast<unsigned>(stepIndex + 1), x, y, r.x, r.y, r.w, r.h, validationHit ? "yes" : "no");
            DisplayCalibrationValidation(stepIndex, true, validationHit);
        }
    }

    // Handle a stable release and advance or restart the state machine.
    void onRelease(void) {
        if (phase == PHASE_CORNER) {
            if (!sampling || sampleCount == 0 || !holdReady) {
                sampling = false;
                holdReady = false;
                DisplayCalibrationTarget(stepIndex, false, false);
                return;
            }

            cornerRaw[stepIndex][0] = rawSumX / sampleCount;
            cornerRaw[stepIndex][1] = rawSumY / sampleCount;
            LOG_I(TAG, "corner %u raw %u,%u samples %u", static_cast<unsigned>(stepIndex + 1), cornerRaw[stepIndex][0], cornerRaw[stepIndex][1], static_cast<unsigned>(sampleCount));
            sampling = false;
            holdReady = false;
            sampleCount = 0;
            stepIndex++;
            stepStartMs = millis();

            if (stepIndex >= 4) {
                computeCandidate();
                LOG_I(TAG, "candidate P0 %u,%u P1 %u,%u P2 %u,%u P3 %u,%u", candidate[0], candidate[1], candidate[2], candidate[3], candidate[4], candidate[5], candidate[6], candidate[7]);
                DisplayTouchCalibrationApply(candidate);
                phase = PHASE_VALIDATE;
                stepIndex = 0;
                DisplayCalibrationValidation(0, false, false);
            } else {
                DisplayCalibrationTarget(stepIndex, false, false);
            }
            return;
        }

        if (phase == PHASE_VALIDATE) {
            if (validationHit) {
                LOG_I(TAG, "validate %u hit", static_cast<unsigned>(stepIndex + 1));
                validationHit = false;
                stepIndex++;
                stepStartMs = millis();

                if (stepIndex >= 4) {
                    save();
                } else {
                    DisplayCalibrationValidation(stepIndex, false, false);
                }
                return;
            }

            LOG_W(TAG, "validate %u miss, restarting", static_cast<unsigned>(stepIndex + 1));
            DisplayTouchCalibrationApply(previous);
            DisplayCalibrationMessage("Missed the button", "restarting");
            phase = PHASE_CORNER;
            stepIndex = 0;
            sampleCount = 0;
            sampling = false;
            holdReady = false;
            restartPending = true;
            restartAtMs = millis();
        }
    }
}

void touch_calibration::setup(void) {
    DisplayTouchCalibrationFallback(current);

    Preferences preferences;
    if (preferences.begin(web_config::NVS_NAMESPACE, true)) {
        if (preferences.isKey(NVS_KEY_TOUCH_CAL) && preferences.getBytesLength(NVS_KEY_TOUCH_CAL) == sizeof(current)) {
            preferences.getBytes(NVS_KEY_TOUCH_CAL, current, sizeof(current));
            hasStored = true;
        }
        preferences.end();
    }

    // Apply the measured four-corner fallback even without a stored
    // calibration, so a fresh device starts with the measured default instead
    // of the coarse panel config rectangle.
    DisplayTouchCalibrationApply(current);
}

void touch_calibration::loop(void) {
    if (phase == PHASE_IDLE) {
        return;
    }

    uint32_t now = millis();

    if (restartPending) {
        if (now - restartAtMs < RESTART_DELAY_MS) {
            return;
        }
        restartPending = false;
        stepStartMs = now;
        DisplayCalibrationTarget(0, false, false);
        return;
    }

    if (now - stepStartMs > STEP_TIMEOUT_MS) {
        LOG_W(TAG, "step timeout, aborting");
        abort();
        return;
    }

    if (now - lastSampleMs < TOUCH_SAMPLE_MS) {
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
            onPress(x, y);
        } else {
            onRelease();
        }
    }

    // Sample only while the finger is actually down; the debounce window after
    // release would otherwise add zero raw values to the average.
    if (down && sampling) {
        rawSumX += rawX;
        rawSumY += rawY;
        sampleCount++;

        if (!holdReady && now - pressStartMs >= HOLD_MS) {
            holdReady = true;
            DisplayCalibrationTarget(stepIndex, true, true);
        }
    }
}

bool touch_calibration::active(void) {
    return phase != PHASE_IDLE;
}

bool touch_calibration::start(void) {
    if (phase != PHASE_IDLE) {
        return false;
    }

    memcpy(previous, current, sizeof(current));
    phase = PHASE_CORNER;
    stepIndex = 0;
    sampleCount = 0;
    sampling = false;
    holdReady = false;
    validationHit = false;
    restartPending = false;
    // Swallow the press that started the calibration, so the first target
    // waits for a fresh tap.
    rawDown = true;
    stableDown = true;
    stepStartMs = millis();
    lastSampleMs = millis();
    rawChangeMs = millis();

    DisplayCalibrationBegin();
    DisplayCalibrationTarget(0, false, false);
    LOG_I(TAG, "calibration started");
    return true;
}

void touch_calibration::abort(void) {
    if (phase == PHASE_IDLE) {
        return;
    }

    DisplayTouchCalibrationApply(previous);
    phase = PHASE_IDLE;
    DisplayCalibrationEnd();
    LOG_I(TAG, "calibration aborted");
}

void touch_calibration::reset(void) {
    if (phase != PHASE_IDLE) {
        abort();
    }

    Preferences preferences;

    if (preferences.begin(web_config::NVS_NAMESPACE, false)) {
        preferences.remove(NVS_KEY_TOUCH_CAL);
        preferences.end();
    }

    DisplayTouchCalibrationFallback(current);
    DisplayTouchCalibrationApply(current);
    hasStored = false;
    LOG_I(TAG, "calibration reset");
}

bool touch_calibration::stored(void) {
    return hasStored;
}

size_t touch_calibration::valueCount(void) {
    return 8;
}

uint16_t touch_calibration::valueAt(size_t index) {
    if (index >= 8) {
        return 0;
    }

    return current[index];
}

const char *touch_calibration::stepName(void) {
    static char buffer[16];

    if (phase == PHASE_IDLE) {
        return "idle";
    }

    if (phase == PHASE_CORNER) {
        snprintf(buffer, sizeof(buffer), "corner %u/4", stepIndex + 1);
    } else {
        snprintf(buffer, sizeof(buffer), "validate %u/4", stepIndex + 1);
    }

    return buffer;
}
