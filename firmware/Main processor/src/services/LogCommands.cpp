/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "LogCommands.h"

#include <cstring>

#include "CliFormat.h"
#include "Log.h"

namespace {
    const char *levelName(logger::Level level) {
        switch (level) {
            case logger::NONE:
                return "none";
            case logger::ERROR:
                return "error";
            case logger::WARN:
                return "warn";
            case logger::INFO:
                return "info";
            case logger::DEBUG:
                return "debug";
            case logger::VERBOSE:
                return "verbose";
            default:
                return "unknown";
        }
    }

    bool parseLevel(const char *text, logger::Level &level) {
        if (strcmp(text, "none") == 0) {
            level = logger::NONE;
        } else if (strcmp(text, "error") == 0) {
            level = logger::ERROR;
        } else if (strcmp(text, "warn") == 0) {
            level = logger::WARN;
        } else if (strcmp(text, "info") == 0) {
            level = logger::INFO;
        } else if (strcmp(text, "debug") == 0) {
            level = logger::DEBUG;
        } else if (strcmp(text, "verbose") == 0) {
            level = logger::VERBOSE;
        } else {
            return false;
        }

        return true;
    }

    void cmdLog(cli::SerialCli &cli, int argc, char **argv) {
        logger::Level level;

        if (argc == 1) {
            cli_format::keyText(cli, "LEVEL", levelName(logger::level()));
            return;
        }

        if (argc == 2) {
            if (!parseLevel(argv[1], level)) {
                cli.write("Usage: log [<level>]\r\n");
                return;
            }

            logger::setLevel(level);
            cli_format::keyText(cli, "LEVEL", levelName(level));
            return;
        }

        cli.write("Usage: log [<level>]\r\n");
    }
}

namespace log_commands {
    void registerCommands(cli::SerialCli &cli) {
        cli.registerCommand({"log", [&cli](int argc, char **argv) { cmdLog(cli, argc, argv); }, "log level: log, log <none|error|warn|info|debug|verbose>"});
    }
}
