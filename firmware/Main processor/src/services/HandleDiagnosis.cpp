/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "HandleDiagnosis.h"

#include <Arduino.h>

#include "HeatController.h"

namespace {
    // An open NTC reads the 3.3 V rail; a connected NTC reads about 1.8 V.
    constexpr float NTC_OPEN_VOLTAGE = 2.5f;
    // The 24 V rail reads about 24 V when present and about 0 V when absent.
    constexpr float SUPPLY_PRESENT_VOLTAGE = 5.0f;
    // The floating thermocouple input couples the 24 V rail to about 3 V.
    constexpr float THERMOCOUPLE_OPEN_VOLTAGE = 1.5f;
    // The heat controller refreshes the iron current only above this duty.
    constexpr float DUTY_FRESH = 0.2f;
    // A connected heater draws about 3 A; an open heater draws 0 A.
    constexpr float IRON_CURRENT_MIN = 0.1f;
    // The heat controller refreshes the iron current one cycle after the
    // heater turns on, so the current check waits for a fresh measurement.
    constexpr uint32_t CURRENT_FRESH_MS = 1000;

    uint32_t heaterOnSinceMs = 0;
    bool heaterWasOn = false;
}

// The web UI owns the long sentences; keep app.js in sync with these texts.
HandleDiagnosis HandleDiagnosisClassify(void) {
    if (!HeatController.linkUp()) {
        return {HandleState::LOSS, "HW fault", "Heat ctrl absent"};
    }

    if (HeatController.getVoltageNTC() > NTC_OPEN_VOLTAGE) {
        return {HandleState::NO_HANDLE, "No handle", "Connect T12 handle"};
    }

    if (HeatController.getSupplyVoltage() < SUPPLY_PRESENT_VOLTAGE) {
        return {HandleState::SUPPLY_OFF, "24V is off", "Switch 24V on"};
    }

    bool heaterOn = HeatController.getHeaterState() == HEATER_STATE::ON;

    if (heaterOn && !heaterWasOn) {
        heaterOnSinceMs = millis();
    }
    heaterWasOn = heaterOn;

    float rawThermocouple = HeatController.getVoltageThermocouple() * HeatController.getGainHeaterTemperatureAmplifier();
    bool thermocoupleOpen = rawThermocouple > THERMOCOUPLE_OPEN_VOLTAGE;
    bool currentFresh = heaterOn && millis() - heaterOnSinceMs >= CURRENT_FRESH_MS;
    bool currentMissing = currentFresh && HeatController.getHeaterDuty() > DUTY_FRESH && HeatController.getCurrentIronOn() < IRON_CURRENT_MIN;

    if (thermocoupleOpen || currentMissing) {
        return {HandleState::NO_TIP, "No T12 tip", "Press tip in firmly"};
    }

    return {HandleState::OK, nullptr, nullptr};
}

const char *HandleStateName(HandleState state) {
    switch (state) {
        case HandleState::OK:
            return "OK";
        case HandleState::LOSS:
            return "LOSS";
        case HandleState::NO_HANDLE:
            return "NO_HANDLE";
        case HandleState::NO_TIP:
            return "NO_TIP";
        case HandleState::SUPPLY_OFF:
            return "SUPPLY_OFF";
    }

    return "UNKNOWN";
}
