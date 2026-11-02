/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <stdint.h>

/*
 * Snapshot of the values the heating cycle produces. The CLI reads this on
 * demand; nothing is printed from the cycle itself.
 */
struct HeaterStatus {
    float vNTC;
    float tAmbientK;
    float tAmbientC;
    float vThermocouple;
    float tTipC;
    float tTargetC;
    float vVINHeatOff;
    float vVINHeatOn;
    float iIronPowerOff;
    float iIronPowerOn;
    float rTip;
    float rSupply;
    float heatfactor;
    float pTerm;
    float iTerm;
    float dTerm;
    float kp;
    float ki;
    float kd;
    uint32_t adcReadMs;
    uint32_t cycleCount;
    uint32_t lastCycleMs;
    uint32_t pwmDuty;
    uint32_t pwmPeriod;
};

class HeaterClass {
public:
    HeaterClass();
    void init(void);
    void loop(void);
    void setInhibit(bool inhibit);
    void setGains(float kp, float ki, float kd);
    HeaterStatus status(void);

private:
};

extern HeaterClass Heater;
