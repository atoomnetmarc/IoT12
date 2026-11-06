/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "StatsCommands.h"

#include <cstring>

#include "CliFormat.h"
#include "LifetimeStats.h"

namespace {
    void cmdStats(cli::SerialCli &cli, int argc, char **argv) {
        if (argc >= 2) {
            if (strcmp(argv[1], "reset") == 0) {
                lifetime_stats::reset();
            } else if (strcmp(argv[1], "flush") == 0) {
                lifetime_stats::flush();
            } else {
                cli.write("Usage: stats [flush|reset]\r\n");
                return;
            }
        }

        cli_format::keyUInt(cli, "BOOTS", lifetime_stats::bootCount());
        cli_format::keyUInt(cli, "TOTAL", lifetime_stats::totalSeconds());
        cli_format::keyUInt(cli, "SESSION", lifetime_stats::sessionSeconds());
    }
}

namespace stats_commands {
    void registerCommands(cli::SerialCli &cli) {
        cli.registerCommand({"stats", [&cli](int argc, char **argv) { cmdStats(cli, argc, argv); }, "lifetime counters; stats flush|reset"});
    }
}
