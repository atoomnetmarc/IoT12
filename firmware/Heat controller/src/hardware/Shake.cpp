/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "Shake.h"

#include <Arduino.h>
#include <cmath>

#include "AdcInternal.h"
#include "Setting.h"
#include "config.h"

namespace {
    constexpr uint32_t SAMPLE_MS = 10;
    constexpr uint32_t DEBOUNCE_MS = 20;
    constexpr float THRESHOLD_V = 2.5f;
}

ShakeClass Shake;

void ShakeClass::init(void) {
    // Both SHAKE pins are tied together; configure both as inputs without pull.
    pinMode(PIN_SHAKE, INPUT);
    pinMode(A2, INPUT);

    _voltage = ADCInternal.readChannelVdd(PIN_SHAKE);

    for (uint8_t i = 0; i < MEDIAN_SAMPLES; i++) {
        _samples[i] = _voltage;
    }

    _sampleIndex = 0;
    _lastRaw = _voltage >= THRESHOLD_V;
    _debounced = _lastRaw;
    _lastSampleMs = millis();
    _lastRawChangeMs = millis();
    _lastMotionMs = millis();
    _expired = false;
}

// Returns the median of the sample buffer, so a single spike does not change
// the level.
float ShakeClass::_median(void) const {
    float sorted[MEDIAN_SAMPLES];

    for (uint8_t i = 0; i < MEDIAN_SAMPLES; i++) {
        sorted[i] = _samples[i];
    }

    for (uint8_t i = 1; i < MEDIAN_SAMPLES; i++) {
        float value = sorted[i];
        int8_t j = static_cast<int8_t>(i) - 1;

        while (j >= 0 && sorted[j] > value) {
            sorted[j + 1] = sorted[j];
            j--;
        }

        sorted[j + 1] = value;
    }

    return sorted[MEDIAN_SAMPLES / 2];
}

void ShakeClass::loop(void) {
    uint32_t now = millis();

    if ((now - _lastSampleMs) < SAMPLE_MS) {
        return;
    }

    _lastSampleMs = now;
    _voltage = ADCInternal.readChannelVdd(PIN_SHAKE);
    _samples[_sampleIndex] = _voltage;
    _sampleIndex = (_sampleIndex + 1) % MEDIAN_SAMPLES;
    bool raw = _median() >= THRESHOLD_V;

    if (raw != _lastRaw) {
        _lastRaw = raw;
        _lastRawChangeMs = now;

        if (!_expired && (now - _lastMotionMs) >= DEBOUNCE_MS) {
            _lastMotionMs = now;
        }
    }

    if ((now - _lastRawChangeMs) >= DEBOUNCE_MS) {
        _debounced = raw;
    }

    if (!_expired && (now - _lastMotionMs) >= static_cast<uint32_t>(Setting.shakeOffMs)) {
        _expired = true;
    }
}

void ShakeClass::reset(void) {
    _lastMotionMs = millis();
    _expired = false;
}

bool ShakeClass::rawLevel(void) const {
    return _lastRaw;
}

bool ShakeClass::debouncedLevel(void) const {
    return _debounced;
}

float ShakeClass::voltage(void) const {
    return _voltage;
}

uint32_t ShakeClass::lastMotionAgeMs(void) const {
    return millis() - _lastMotionMs;
}

SHAKE_PHASE ShakeClass::phase(void) const {
    if (_expired) {
        return SHAKE_PHASE::EXPIRED;
    }

    uint32_t elapsed = millis() - _lastMotionMs;

    if (elapsed < static_cast<uint32_t>(Setting.shakeStage1Ms)) {
        return SHAKE_PHASE::ACTIVE;
    }

    if (elapsed < static_cast<uint32_t>(Setting.shakeStage2Ms)) {
        return SHAKE_PHASE::RAMP;
    }

    return SHAKE_PHASE::STANDBY2;
}

bool ShakeClass::standbyActive(void) const {
    SHAKE_PHASE current = phase();

    return current == SHAKE_PHASE::RAMP || current == SHAKE_PHASE::STANDBY2;
}

bool ShakeClass::expired(void) const {
    return _expired;
}

float ShakeClass::targetTemperature(void) const {
    float setpoint = Setting.targetTemperatureHeater;
    float first = fminf(setpoint, Setting.standbyTemperatureHeater);
    float second = fminf(setpoint, Setting.standbyTemperatureHeater2);
    uint32_t elapsed = millis() - _lastMotionMs;
    uint32_t stage1 = static_cast<uint32_t>(Setting.shakeStage1Ms);
    uint32_t stage2 = static_cast<uint32_t>(Setting.shakeStage2Ms);

    if (elapsed < stage1) {
        return setpoint;
    }

    if (stage2 <= stage1 || elapsed >= stage2) {
        return second;
    }

    float fraction = static_cast<float>(elapsed - stage1) / static_cast<float>(stage2 - stage1);

    return first + (second - first) * fraction;
}
