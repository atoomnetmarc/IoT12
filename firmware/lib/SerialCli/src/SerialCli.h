/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <cstddef>
#include <functional>

/*
 * Compile-time budget. Each project overrides these through build_flags so the
 * command table and the input buffer fit the target's RAM.
 */
#ifndef CLI_MAX_COMMANDS
#define CLI_MAX_COMMANDS 32
#endif

#ifndef CLI_INPUT_BUFFER_SIZE
#define CLI_INPUT_BUFFER_SIZE 256
#endif

namespace cli {
    struct CommandDef {
        const char *name;
        std::function<void(int argc, char **argv)> handler;
        const char *help;
    };

    using OutputCallback = std::function<void(const char *data, size_t len)>;

    /*
     * Push-driven command line. The owner feeds received characters with
     * feed() and supplies an output callback; the class never touches a UART
     * itself and never blocks.
     */
    class SerialCli {
    public:
        static constexpr size_t MAX_COMMANDS = CLI_MAX_COMMANDS;
        static constexpr size_t INPUT_BUFFER_SIZE = CLI_INPUT_BUFFER_SIZE;

        explicit SerialCli(OutputCallback onOutput);

        bool registerCommand(const CommandDef &cmd);
        void feed(char c);

        void write(const char *text);
        void writef(const char *format, ...) __attribute__((format(printf, 2, 3)));

        size_t commandCount(void) const;
        const CommandDef *commandAt(size_t index) const;

    private:
        OutputCallback _onOutput;
        CommandDef _commands[MAX_COMMANDS];
        size_t _commandCount = 0;
        char _buffer[INPUT_BUFFER_SIZE];
        size_t _pos = 0;

        void dispatch(char *line);
        void cmdHelp(int argc, char **argv);
    };
}
