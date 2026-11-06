/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "Log.h"

#include <cstdarg>
#include <cstdio>

namespace logger {
    namespace {
        Sink _sink = nullptr;
        TimeSource _timeSource = nullptr;
        Level _level = INFO;

        char levelChar(Level level) {
            switch (level) {
                case ERROR:
                    return 'E';
                case WARN:
                    return 'W';
                case INFO:
                    return 'I';
                case DEBUG:
                    return 'D';
                case VERBOSE:
                    return 'V';
                default:
                    return '?';
            }
        }
    }

    void setSink(Sink sink) {
        _sink = sink;
    }

    void setTimeSource(TimeSource source) {
        _timeSource = source;
    }

    void setLevel(Level level) {
        _level = level;
    }

    Level level(void) {
        return _level;
    }

    void write(Level level, const char *tag, const char *format, ...) {
        if (level > _level || _sink == nullptr) {
            return;
        }

        char buffer[160];

        unsigned long now = _timeSource != nullptr ? _timeSource() : 0UL;
        int len = snprintf(buffer, sizeof(buffer), "%c (%lu) %s: ", levelChar(level), now, tag != nullptr ? tag : "?");
        if (len <= 0) {
            return;
        }

        size_t pos = static_cast<size_t>(len) < sizeof(buffer) ? static_cast<size_t>(len) : sizeof(buffer) - 1;

        va_list args;
        va_start(args, format);
        int written = vsnprintf(buffer + pos, sizeof(buffer) - pos, format, args);
        va_end(args);

        if (written > 0) {
            pos += static_cast<size_t>(written);
            if (pos >= sizeof(buffer)) {
                pos = sizeof(buffer) - 1;
            }
        }

        if (pos + 2 < sizeof(buffer)) {
            buffer[pos++] = '\r';
            buffer[pos++] = '\n';
        }

        buffer[pos] = '\0';
        _sink(buffer, pos);
    }
}
