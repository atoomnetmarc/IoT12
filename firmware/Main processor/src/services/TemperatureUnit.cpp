/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "TemperatureUnit.h"

#include <Arduino.h>
#include <Preferences.h>

#include <cstdio>
#include <cstring>

#include "WebConfigStore.h"

namespace {
    constexpr const char *NVS_KEY = "temp_unit";
    const temperature::Unit *activeUnit = nullptr;
}

namespace temperature_unit {
    // Loads the stored unit id and falls back to Celsius when it is unknown.
    void setup(void) {
        char id[16] = {0};

        Preferences preferences;
        if (preferences.begin(web_config::NVS_NAMESPACE, true)) {
            String stored = preferences.getString(NVS_KEY, "");
            preferences.end();
            snprintf(id, sizeof(id), "%s", stored.c_str());
        }

        activeUnit = temperature::find(id);
        if (activeUnit == nullptr) {
            activeUnit = temperature::celsius();
        }
    }

    // Returns the active unit, or Celsius before setup has run.
    const temperature::Unit *active(void) {
        return activeUnit != nullptr ? activeUnit : temperature::celsius();
    }

    // Returns the stable id of the active unit.
    const char *id(void) {
        return active()->id;
    }

    // Stores a unit id and makes it active; returns false for an unknown id.
    bool set(const char *id) {
        const temperature::Unit *unit = temperature::find(id);
        if (unit == nullptr) {
            return false;
        }

        Preferences preferences;
        if (!preferences.begin(web_config::NVS_NAMESPACE, false)) {
            return false;
        }

        size_t written = preferences.putString(NVS_KEY, unit->id);
        preferences.end();

        if (written != strlen(unit->id)) {
            return false;
        }

        activeUnit = unit;
        return true;
    }
}
