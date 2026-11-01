/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <cstddef>
#include <cstdint>

#ifndef EVENTLOG_CAPACITY
#define EVENTLOG_CAPACITY 32
#endif

namespace eventlog {
    /*
     * One-shot events only. Nothing in this list is periodic: an event is
     * recorded when something changes, never on a timer.
     */
    enum Code : uint8_t {
        BOOT = 0,
        RESET_CAUSE,
        FAULT,
        LINK_TIMEOUT,
        HEATER_ON,
        HEATER_OFF,
        SETTINGS_SAVE,
        SETTINGS_LOAD,
        SETTINGS_DEFAULT,
        FLASH_ERROR,
        FS_ERROR,
        WIFI_CONNECT,
        WIFI_DISCONNECT,
        OTA_START,
        OTA_END,
        OTA_ERROR,
        BUTTON,
        CODE_COUNT
    };

    struct Record {
        uint32_t ms;
        int32_t value;
        Code code;
    };

    using Sink = void (*)(const char *data, size_t len);
    using TimeSource = unsigned long (*)(void);

    /*
     * Fixed-size ring buffer. Recording an event never prints unless live
     * printing was explicitly enabled, and even then it is one line per event.
     */
    class EventLog {
    public:
        static constexpr size_t CAPACITY = EVENTLOG_CAPACITY;

        void setTimeSource(TimeSource source);
        void setSink(Sink sink);

        void record(Code code, int32_t value = 0);
        void clear(void);

        void setLive(bool enabled);
        bool live(void) const;

        size_t count(void) const;
        const Record &at(size_t index) const;

        static const char *name(Code code);

    private:
        Record _records[CAPACITY];
        size_t _count = 0;
        size_t _next = 0;
        bool _live = false;
        TimeSource _timeSource = nullptr;
        Sink _sink = nullptr;
    };

    extern EventLog Log;
}
