/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "ScreenCommands.h"

#include <cstring>

#include "CliFormat.h"
#include "display.h"

namespace {
    void cmdScreen(cli::SerialCli &cli, int argc, char **argv) {
        if (argc >= 2) {
            if (strcmp(argv[1], "next") == 0) {
                DisplayScreenNext();
            } else if (strcmp(argv[1], "prev") == 0) {
                DisplayScreenPrev();
            } else if (strcmp(argv[1], "home") == 0) {
                DisplayScreenSet(DISPLAY_SCREEN_HOME);
            } else if (strcmp(argv[1], "network") == 0) {
                DisplayScreenSet(DISPLAY_SCREEN_NETWORK);
            } else if (strcmp(argv[1], "info") == 0) {
                DisplayScreenSet(DISPLAY_SCREEN_INFO);
            } else if (strcmp(argv[1], "touch") == 0) {
                DisplayScreenSet(DISPLAY_SCREEN_TOUCH);
            } else {
                cli.write("Usage: screen [next|prev|home|network|info|touch]\r\n");
                return;
            }
        }

        cli_format::keyText(cli, "SCREEN", DisplayScreenName(DisplayScreenGet()));
        cli_format::keyUInt(cli, "INDEX", static_cast<unsigned long>(DisplayScreenGet()));
        cli_format::keyUInt(cli, "COUNT", static_cast<unsigned long>(DisplayScreenCount()));
    }
}

namespace screen_commands {
    void registerCommands(cli::SerialCli &cli) {
        cli.registerCommand({"screen", [&cli](int argc, char **argv) { cmdScreen(cli, argc, argv); }, "carousel screen; screen next|prev|home|network|info|touch"});
    }
}
