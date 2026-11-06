/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace logger {
    enum Level : uint8_t {
        NONE = 0,
        ERROR,
        WARN,
        INFO,
        DEBUG,
        VERBOSE,
    };

    using Sink = void (*)(const char *data, size_t len);
    using TimeSource = unsigned long (*)(void);

    void setSink(Sink sink);
    void setTimeSource(TimeSource source);

    void setLevel(Level level);
    Level level(void);

    /*
     * One line per call: "I (12345) Tag: message\r\n". Nothing is printed when
     * the level is above the configured one, and nothing is buffered.
     */
    void write(Level level, const char *tag, const char *format, ...) __attribute__((format(printf, 3, 4)));
}

#define LOG_E(tag, ...) logger::write(logger::ERROR, tag, __VA_ARGS__)
#define LOG_W(tag, ...) logger::write(logger::WARN, tag, __VA_ARGS__)
#define LOG_I(tag, ...) logger::write(logger::INFO, tag, __VA_ARGS__)
#define LOG_D(tag, ...) logger::write(logger::DEBUG, tag, __VA_ARGS__)
#define LOG_V(tag, ...) logger::write(logger::VERBOSE, tag, __VA_ARGS__)
