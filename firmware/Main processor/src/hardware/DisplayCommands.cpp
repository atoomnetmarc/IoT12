/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "DisplayCommands.h"

#include "CliFormat.h"
#include "display.h"

namespace {
    void cmdDisplay(cli::SerialCli &cli, int argc, char **argv) {
        (void)argc;
        (void)argv;

        cli_format::keyText(cli, "DRIVER", DisplayDriverName());
        cli_format::keyText(cli, "RESOLUTION", "0x0");
        cli_format::keyText(cli, "BACKLIGHT", "n/a");
    }
}

namespace display_commands {
    void registerCommands(cli::SerialCli &cli) {
        cli.registerCommand({"display", [&cli](int argc, char **argv) { cmdDisplay(cli, argc, argv); }, "display driver and panel state"});
    }
}
