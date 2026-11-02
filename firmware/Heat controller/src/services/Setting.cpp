/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "Setting.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <cstring>

#include "EventLog.h"
#include "Storage.h"

namespace {
    constexpr uint32_t FILE_CAP = 512;

    struct Field {
        const char *name;
        float SettingClass::*member;
    };

    const Field FIELDS[] = {
        {"targetTemperatureHeater", &SettingClass::targetTemperatureHeater},
        {"gainVSupplyResistorDivider", &SettingClass::gainVSupplyResistorDivider},
        {"gainHeaterTemperatureAmplifier", &SettingClass::gainHeaterTemperatureAmplifier},
        {"gainHeaterCurrentAmplifier", &SettingClass::gainHeaterCurrentAmplifier},
        {"resistanceHeaterShunt", &SettingClass::resistanceHeaterShunt},
        {"resistanceNTCPullUp", &SettingClass::resistanceNTCPullUp},
        {"thermocoupleInputOffsetVoltage", &SettingClass::thermocoupleInputOffsetVoltage},
    };

    /*
     * Reads a settings file into a document. Returns false when the volume is
     * not mounted, the file is missing, or the content is not a JSON object.
     */
    bool readDocument(const char *path, JsonDocument &doc) {
        if (!Storage.mounted()) {
            return false;
        }

        lfs_file_t file;

        if (lfs_file_open(&Storage.fs(), &file, path, LFS_O_RDONLY) != LFS_ERR_OK) {
            return false;
        }

        char buffer[FILE_CAP];
        lfs_ssize_t count = lfs_file_read(&Storage.fs(), &file, buffer, sizeof(buffer));
        lfs_file_close(&Storage.fs(), &file);

        if (count <= 0) {
            return false;
        }

        if (deserializeJson(doc, buffer, static_cast<size_t>(count)) != DeserializationError::Ok) {
            return false;
        }

        return doc.is<JsonObject>();
    }
}

SettingClass Setting;

void SettingClass::init(void) {
    _load(false);
}

void SettingClass::loop(void) {
    static unsigned long saveTimer = 0;

    if (_saveRequested) {
        _saveRequested = false;
        saveTimer = millis() + 500; // (re)schedule save over 0.5 seconds in case more settings are changed.
    }

    if (saveTimer > 0 && saveTimer < millis()) {
        saveTimer = 0;
        _save();
    }
}

void SettingClass::defaults(void) {
    targetTemperatureHeater = 373.15f;

    // Power supply voltage has been divided by a 100K in series with a 4K7 resistor.
    gainVSupplyResistorDivider = 0.04489f;

    gainHeaterTemperatureAmplifier = 106.1f;
    gainHeaterCurrentAmplifier = 50.4f;

    resistanceHeaterShunt = 0.005f;
    resistanceNTCPullUp = 10000.0f;

    // INA826 input offset voltage 40uV typical to 150uV maximum.
    thermocoupleInputOffsetVoltage = 0.00004f;
}

void SettingClass::load(void) {
    _load(false);
}

void SettingClass::save(void) {
    _saveRequested = true;
}

void SettingClass::saveNow(void) {
    _saveRequested = false;
    _save();
}

bool SettingClass::get(const char *key, float &value) const {
    for (const Field &field : FIELDS) {
        if (strcmp(field.name, key) == 0) {
            value = this->*(field.member);
            return true;
        }
    }

    return false;
}

bool SettingClass::set(const char *key, float value) {
    for (const Field &field : FIELDS) {
        if (strcmp(field.name, key) == 0) {
            this->*(field.member) = value;
            return true;
        }
    }

    return false;
}

size_t SettingClass::keyCount(void) const {
    return sizeof(FIELDS) / sizeof(FIELDS[0]);
}

const char *SettingClass::keyAt(size_t index) const {
    if (index >= keyCount()) {
        return nullptr;
    }

    return FIELDS[index].name;
}

uint32_t SettingClass::fileSize(const char *path) {
    if (!Storage.mounted()) {
        return 0;
    }

    lfs_info info;

    if (lfs_stat(&Storage.fs(), path, &info) != LFS_ERR_OK) {
        return 0;
    }

    return info.size;
}

bool SettingClass::fileValid(const char *path) {
    JsonDocument doc;

    return readDocument(path, doc);
}

// Copies the current settings file to the backup. A missing primary is not an
// error: the first save has nothing to back up.
bool SettingClass::_backup(void) {
    lfs_info info;
    int statError = lfs_stat(&Storage.fs(), setting_files::PRIMARY, &info);

    if (statError == LFS_ERR_NOENT) {
        return true;
    }

    if (statError != LFS_ERR_OK || info.size > FILE_CAP) {
        return false;
    }

    lfs_file_t source;

    if (lfs_file_open(&Storage.fs(), &source, setting_files::PRIMARY, LFS_O_RDONLY) != LFS_ERR_OK) {
        return false;
    }

    char buffer[FILE_CAP];
    lfs_ssize_t count = lfs_file_read(&Storage.fs(), &source, buffer, sizeof(buffer));
    lfs_file_close(&Storage.fs(), &source);

    if (count != static_cast<lfs_ssize_t>(info.size)) {
        return false;
    }

    lfs_file_t target;

    if (lfs_file_open(&Storage.fs(), &target, setting_files::BACKUP, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC) != LFS_ERR_OK) {
        return false;
    }

    lfs_ssize_t written = lfs_file_write(&Storage.fs(), &target, buffer, static_cast<lfs_size_t>(count));
    int closeError = lfs_file_close(&Storage.fs(), &target);

    return written == count && closeError == LFS_ERR_OK;
}

void SettingClass::_save(void) {
    if (!Storage.mounted()) {
        eventlog::Log.record(eventlog::FS_ERROR, 0);
        return;
    }

    if (!_backup()) {
        eventlog::Log.record(eventlog::FS_ERROR, 0);
        return;
    }

    JsonDocument doc;

    for (const Field &field : FIELDS) {
        doc[field.name] = this->*(field.member);
    }

    char buffer[FILE_CAP];
    size_t length = serializeJson(doc, buffer, sizeof(buffer));

    if (length == 0 || length >= sizeof(buffer)) {
        eventlog::Log.record(eventlog::FS_ERROR, 0);
        return;
    }

    lfs_file_t file;

    if (lfs_file_open(&Storage.fs(), &file, setting_files::PRIMARY, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC) != LFS_ERR_OK) {
        eventlog::Log.record(eventlog::FS_ERROR, 0);
        return;
    }

    lfs_ssize_t written = lfs_file_write(&Storage.fs(), &file, buffer, length);
    int closeError = lfs_file_close(&Storage.fs(), &file);

    if (written != static_cast<lfs_ssize_t>(length) || closeError != LFS_ERR_OK) {
        eventlog::Log.record(eventlog::FS_ERROR, 0);
        return;
    }

    eventlog::Log.record(eventlog::SETTINGS_SAVE, static_cast<int32_t>(length));
}

// Applies the keys present in a file; a missing key keeps its default.
bool SettingClass::_applyFile(const char *path) {
    JsonDocument doc;

    if (!readDocument(path, doc)) {
        return false;
    }

    for (const Field &field : FIELDS) {
        JsonVariant value = doc[field.name];

        if (value.is<float>()) {
            this->*(field.member) = value.as<float>();
        }
    }

    return true;
}

void SettingClass::_load(bool useDefaults) {
    defaults();

    bool loaded = false;

    if (!useDefaults) {
        loaded = _applyFile(setting_files::PRIMARY);

        if (!loaded) {
            loaded = _applyFile(setting_files::BACKUP);
        }
    }

    eventlog::Log.record(loaded ? eventlog::SETTINGS_LOAD : eventlog::SETTINGS_DEFAULT, 0);
}
