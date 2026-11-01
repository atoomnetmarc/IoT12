/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "SerialCli.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <utility>

namespace cli {
    SerialCli::SerialCli(OutputCallback onOutput)
        : _onOutput(std::move(onOutput)) {
        if (!_onOutput) {
            _onOutput = [](const char *data, size_t len) {
                (void)data;
                (void)len;
            };
        }

        _buffer[0] = '\0';
    }

    bool SerialCli::registerCommand(const CommandDef &cmd) {
        if (cmd.name == nullptr || !cmd.handler) {
            return false;
        }

        /* Re-registering a name replaces the earlier definition. */
        for (size_t i = 0; i < _commandCount; ++i) {
            if (strcmp(_commands[i].name, cmd.name) == 0) {
                _commands[i] = cmd;
                return true;
            }
        }

        if (_commandCount >= MAX_COMMANDS) {
            return false;
        }

        _commands[_commandCount++] = cmd;
        return true;
    }

    void SerialCli::feed(char c) {
        if (c == 0x7F || c == '\b') {
            if (_pos > 0) {
                _pos--;
                _onOutput("\b \b", 3);
            }
            return;
        }

        if (c == '\n' || c == '\r') {
            /* '\r\n' dispatches once: the second terminator finds an empty buffer. */
            if (_pos > 0) {
                _buffer[_pos] = '\0';
                _pos = 0;
                _onOutput("\r\n", 2);
                dispatch(_buffer);
            }
            return;
        }

        if (_pos < INPUT_BUFFER_SIZE - 1) {
            _buffer[_pos++] = c;
            _onOutput(&c, 1);
        }
        /* Buffer full: drop the character, never overflow. */
    }

    void SerialCli::dispatch(char *line) {
        while (*line == ' ') {
            line++;
        }

        if (*line == '\0') {
            return;
        }

        /*
         * Tokenize in place. The line buffer is not needed after dispatch, so
         * this costs no extra stack on the SAMD21.
         */
        constexpr size_t MAX_ARGS = 16;
        char *argv[MAX_ARGS];
        int argc = 0;

        char *p = line;
        while (*p != '\0' && argc < static_cast<int>(MAX_ARGS)) {
            while (*p == ' ') {
                p++;
            }

            if (*p == '\0') {
                break;
            }

            argv[argc++] = p;

            while (*p != '\0' && *p != ' ') {
                p++;
            }

            if (*p == ' ') {
                *p = '\0';
                p++;
            }
        }

        if (argc == 0) {
            return;
        }

        if (strcmp(argv[0], "help") == 0) {
            cmdHelp(argc, argv);
            return;
        }

        for (size_t i = 0; i < _commandCount; ++i) {
            if (strcmp(_commands[i].name, argv[0]) == 0) {
                _commands[i].handler(argc, argv);
                return;
            }
        }

        writef("Unknown command: '%s'. Try 'help'.\r\n", argv[0]);
    }

    void SerialCli::write(const char *text) {
        if (text == nullptr) {
            return;
        }

        _onOutput(text, strlen(text));
    }

    void SerialCli::writef(const char *format, ...) {
        /*
         * 160 bytes: a help line is "  " + 18 + " " + help text + "\r\n".
         * At 96 the longest help text lost its "\r\n" and merged with the
         * next line.
         */
        char line[160];

        va_list args;
        va_start(args, format);
        int len = vsnprintf(line, sizeof(line), format, args);
        va_end(args);

        if (len <= 0) {
            return;
        }

        if (static_cast<size_t>(len) >= sizeof(line)) {
            len = static_cast<int>(sizeof(line) - 1);
        }

        _onOutput(line, static_cast<size_t>(len));
    }

    size_t SerialCli::commandCount(void) const {
        return _commandCount;
    }

    const CommandDef *SerialCli::commandAt(size_t index) const {
        if (index >= _commandCount) {
            return nullptr;
        }

        return &_commands[index];
    }

    void SerialCli::cmdHelp(int argc, char **argv) {
        (void)argc;
        (void)argv;

        write("=== Serial commands ===\r\n");

        for (size_t i = 0; i < _commandCount; ++i) {
            writef("  %-18s %s\r\n", _commands[i].name, _commands[i].help != nullptr ? _commands[i].help : "");
        }
    }
}
