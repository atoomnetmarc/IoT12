/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <stdint.h>

/*
 * The standby schedule after the last motion. The target follows the setpoint
 * while active, ramps down between the two standby temperatures, holds the
 * second temperature, and then expires.
 */
enum class SHAKE_PHASE : uint8_t {
    ACTIVE,
    RAMP,
    STANDBY2,
    EXPIRED,
};

class ShakeClass {
public:
    void init(void);
    void loop(void);
    void reset(void);

    bool rawLevel(void) const;
    bool debouncedLevel(void) const;
    float voltage(void) const;
    uint32_t lastMotionAgeMs(void) const;
    SHAKE_PHASE phase(void) const;
    bool standbyActive(void) const;
    bool expired(void) const;
    float targetTemperature(void) const;

private:
    enum : uint8_t { MEDIAN_SAMPLES = 5 };

    float _median(void) const;

    bool _debounced = false;
    bool _lastRaw = false;
    float _voltage = 0;
    float _samples[MEDIAN_SAMPLES] = {};
    uint8_t _sampleIndex = 0;
    uint32_t _lastSampleMs = 0;
    uint32_t _lastRawChangeMs = 0;
    uint32_t _lastMotionMs = 0;
    bool _expired = false;
};

extern ShakeClass Shake;
