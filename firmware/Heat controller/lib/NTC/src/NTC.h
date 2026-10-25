/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

class NTC {
private:
    float resistorDividerVoltage;
    float resistorDividerResistance;
    float thermistorResistance;
    float thermistorTemperature;
    float thermistorBValue;

public:
    NTC(void);

    void SetParameters(
        float resistorDividerVoltage,
        float resistorDividerResistance,
        float thermistorResistance,
        float thermistorTemperature,
        float thermistorBValue);

    float GetTemperature(float voltage);
};
