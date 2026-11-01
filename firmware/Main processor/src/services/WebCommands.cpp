/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "WebCommands.h"

#include "CliFormat.h"
#include "webserverc.h"

namespace {
    void cmdWeb(cli::SerialCli &cli, int argc, char **argv) {
        (void)argc;
        (void)argv;

        cli_format::keyText(cli, "RUNNING", WebservercRunning() ? "yes" : "no");
        cli_format::keyUInt(cli, "PORT", 80);
        cli_format::keyUInt(cli, "REQUESTS", WebservercRequestCount());
        cli_format::keyText(cli, "LAST_URI", WebservercLastUri());
    }
}

namespace web_commands {
    void registerCommands(cli::SerialCli &cli) {
        cli.registerCommand({"web", [&cli](int argc, char **argv) { cmdWeb(cli, argc, argv); }, "HTTP server state, request count, last URI"});
    }
}
