/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

/*
 * The settings file and its backup. The primary file wins; the backup is read
 * only when the primary is missing or invalid.
 */
namespace setting_files {
    constexpr const char *PRIMARY = "/settings.json";
    constexpr const char *BACKUP = "/settings.json.bak";
}

enum HEATER_STATE : uint8_t {
    OFF = 0x00,
    ON = 0x01,
};

/*
 * Settings live in /settings.json on the LittleFS volume. The JSON keys are the
 * member names, so the file, the CLI and the tests use one name per setting.
 */
class SettingClass {
public:
    void init(void);
    void loop(void);
    void defaults(void);
    void load(void);
    void save(void);
    void saveNow(void);

    bool get(const char *key, float &value) const;
    bool set(const char *key, float value);

    size_t keyCount(void) const;
    const char *keyAt(size_t index) const;

    uint32_t fileSize(const char *path);
    bool fileValid(const char *path);

    HEATER_STATE heaterState = HEATER_STATE::OFF;

    float targetTemperatureHeater;
    float gainVSupplyResistorDivider;
    float gainHeaterTemperatureAmplifier;
    float gainHeaterCurrentAmplifier;
    float resistanceHeaterShunt;
    float resistanceNTCPullUp;
    float thermocoupleInputOffsetVoltage;
    float standbyTemperatureHeater;
    float standbyTemperatureHeater2;
    float shakeStage1Ms;
    float shakeStage2Ms;
    float shakeOffMs;

private:
    void _save(void);
    void _load(bool useDefaults);
    bool _backup(void);
    bool _applyFile(const char *path);

    bool _saveRequested = false;
};

extern SettingClass Setting;
