/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "UnitCommands.h"

#include <cstring>

#include "CliFormat.h"
#include "Temperature.h"
#include "TemperatureUnit.h"
#include "display.h"

namespace {
    // Prints the active unit and its properties.
    void printActive(cli::SerialCli &cli) {
        const temperature::Unit *unit = temperature_unit::active();

        cli_format::keyText(cli, "UNIT", unit->id);
        cli_format::keyText(cli, "NAME", unit->name);
        cli_format::keyText(cli, "SYMBOL", unit->symbol);
        cli_format::keyUInt(cli, "DECIMALS", unit->decimals);
        cli_format::keyFloat(cli, "STEP", unit->step, 2);
        cli_format::keyInt(cli, "DIRECTION", unit->direction);
        cli_format::keyText(cli, "COMMON", unit->common ? "yes" : "no");
    }

    // Prints every unit id with its name, symbol and common flag.
    void printList(cli::SerialCli &cli) {
        for (size_t i = 0; i < temperature::unitCount(); i++) {
            const temperature::Unit *unit = &temperature::units()[i];
            cli.writef("%s: %s %s%s\r\n", unit->id, unit->name, unit->symbol, unit->common ? " common" : "");
        }
    }

    // Handles `unit`, `unit list` and `unit set <id>`.
    void cmdUnit(cli::SerialCli &cli, int argc, char **argv) {
        if (argc >= 2 && strcmp(argv[1], "list") == 0) {
            printList(cli);
            return;
        }

        if (argc >= 2 && strcmp(argv[1], "set") == 0) {
            if (argc < 3) {
                cli.write("Usage: unit set <id>\r\n");
                return;
            }

            if (!temperature_unit::set(argv[2])) {
                cli.writef("unit: unknown id '%s'\r\n", argv[2]);
                cli.write("Use: unit list\r\n");
                return;
            }

            DisplayRequestDraw();
            printActive(cli);
            return;
        }

        if (argc >= 2) {
            cli.write("Usage: unit [list|set <id>]\r\n");
            return;
        }

        printActive(cli);
    }
}

namespace unit_commands {
    void registerCommands(cli::SerialCli &cli) {
        cli.registerCommand({"unit", [&cli](int argc, char **argv) { cmdUnit(cli, argc, argv); }, "temperature unit: state, unit list, unit set <id>"});
    }
}
