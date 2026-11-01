/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "OtaCommands.h"

#include "CliFormat.h"
#include "config.h"
#include "ota.h"

namespace {
    void cmdOta(cli::SerialCli &cli, int argc, char **argv) {
        (void)argc;
        (void)argv;

        cli_format::keyText(cli, "STATE", OtaStateName(OtaGetState()));
        cli_format::keyUInt(cli, "PROGRESS", OtaGetProgress());
        cli_format::keyInt(cli, "LAST_ERROR", OtaGetLastError());
        cli_format::keyUInt(cli, "PORT", 3232);
        cli_format::keyText(cli, "HOSTNAME", otahostname);
    }
}

namespace ota_commands {
    void registerCommands(cli::SerialCli &cli) {
        cli.registerCommand({"ota", [&cli](int argc, char **argv) { cmdOta(cli, argc, argv); }, "OTA state, progress, last error"});
    }
}
