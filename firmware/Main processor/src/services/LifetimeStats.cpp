/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "LifetimeStats.h"

#include <Arduino.h>
#include <Preferences.h>
#include <esp_timer.h>

#include "WebConfigStore.h"

namespace {
    constexpr const char *NVS_KEY_BOOTS = "boots";
    constexpr const char *NVS_KEY_SECS = "secs";
    constexpr int64_t FLUSH_INTERVAL_US = 15LL * 60LL * 1000000LL;

    uint32_t storedSeconds = 0;
    uint32_t boots = 0;
    int64_t sessionStartUs = 0;
    int64_t lastFlushUs = 0;

    // Seconds since the session started, from the 64-bit timer.
    uint32_t elapsedSeconds(void) {
        return static_cast<uint32_t>((esp_timer_get_time() - sessionStartUs) / 1000000LL);
    }

    // Write both counters in one NVS commit.
    void writeCounters(void) {
        Preferences preferences;

        if (!preferences.begin(web_config::NVS_NAMESPACE, false)) {
            return;
        }

        preferences.putUInt(NVS_KEY_BOOTS, boots);
        preferences.putUInt(NVS_KEY_SECS, storedSeconds);
        preferences.end();
    }
}

void lifetime_stats::setup(void) {
    Preferences preferences;

    if (preferences.begin(web_config::NVS_NAMESPACE, true)) {
        boots = preferences.getUInt(NVS_KEY_BOOTS, 0);
        storedSeconds = preferences.getUInt(NVS_KEY_SECS, 0);
        preferences.end();
    }

    boots++;
    sessionStartUs = esp_timer_get_time();
    lastFlushUs = sessionStartUs;

    writeCounters();
}

void lifetime_stats::loop(void) {
    if (esp_timer_get_time() - lastFlushUs < FLUSH_INTERVAL_US) {
        return;
    }

    flush();
}

void lifetime_stats::flush(void) {
    storedSeconds = totalSeconds();
    sessionStartUs = esp_timer_get_time();
    lastFlushUs = sessionStartUs;

    writeCounters();
}

uint32_t lifetime_stats::bootCount(void) {
    return boots;
}

uint32_t lifetime_stats::totalSeconds(void) {
    return storedSeconds + elapsedSeconds();
}

uint32_t lifetime_stats::sessionSeconds(void) {
    return elapsedSeconds();
}

void lifetime_stats::reset(void) {
    boots = 0;
    storedSeconds = 0;
    sessionStartUs = esp_timer_get_time();
    lastFlushUs = sessionStartUs;

    writeCounters();
}
