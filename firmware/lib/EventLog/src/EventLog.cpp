/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "EventLog.h"

#include <cstdio>

namespace eventlog {
    EventLog Log;

    void EventLog::setTimeSource(TimeSource source) {
        _timeSource = source;
    }

    void EventLog::setSink(Sink sink) {
        _sink = sink;
    }

    void EventLog::record(Code code, int32_t value) {
        Record &entry = _records[_next];
        entry.ms = _timeSource != nullptr ? static_cast<uint32_t>(_timeSource()) : 0;
        entry.value = value;
        entry.code = code;

        _next = (_next + 1) % CAPACITY;
        if (_count < CAPACITY) {
            _count++;
        }

        if (!_live || _sink == nullptr) {
            return;
        }

        char line[64];
        int len = snprintf(line, sizeof(line), "[%10lu] %-16s %ld\r\n", static_cast<unsigned long>(entry.ms), name(code), static_cast<long>(value));
        if (len <= 0) {
            return;
        }

        if (static_cast<size_t>(len) >= sizeof(line)) {
            len = static_cast<int>(sizeof(line) - 1);
        }

        _sink(line, static_cast<size_t>(len));
    }

    void EventLog::clear(void) {
        _count = 0;
        _next = 0;
    }

    void EventLog::setLive(bool enabled) {
        _live = enabled;
    }

    bool EventLog::live(void) const {
        return _live;
    }

    size_t EventLog::count(void) const {
        return _count;
    }

    const Record &EventLog::at(size_t index) const {
        /* index 0 is the oldest record still in the buffer. */
        if (_count < CAPACITY) {
            return _records[index];
        }

        return _records[(_next + index) % CAPACITY];
    }

    const char *EventLog::name(Code code) {
        switch (code) {
            case BOOT:
                return "BOOT";
            case RESET_CAUSE:
                return "RESET_CAUSE";
            case FAULT:
                return "FAULT";
            case LINK_TIMEOUT:
                return "LINK_TIMEOUT";
            case HEATER_ON:
                return "HEATER_ON";
            case HEATER_OFF:
                return "HEATER_OFF";
            case SETTINGS_SAVE:
                return "SETTINGS_SAVE";
            case SETTINGS_LOAD:
                return "SETTINGS_LOAD";
            case SETTINGS_DEFAULT:
                return "SETTINGS_DEFAULT";
            case FLASH_ERROR:
                return "FLASH_ERROR";
            case FS_ERROR:
                return "FS_ERROR";
            case WIFI_CONNECT:
                return "WIFI_CONNECT";
            case WIFI_DISCONNECT:
                return "WIFI_DISCONNECT";
            case OTA_START:
                return "OTA_START";
            case OTA_END:
                return "OTA_END";
            case OTA_ERROR:
                return "OTA_ERROR";
            case BUTTON:
                return "BUTTON";
            default:
                return "UNKNOWN";
        }
    }
}
