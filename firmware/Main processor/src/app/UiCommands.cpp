/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "UiCommands.h"

#include <cstdlib>
#include <cstring>

#include "CliFormat.h"
#include "Ui.h"

namespace {
    void cmdUi(cli::SerialCli &cli, int argc, char **argv) {
        if (argc >= 2 && strcmp(argv[1], "setpoint") == 0) {
            if (argc < 3) {
                cli.write("Usage: ui setpoint <celsius>\r\n");
                return;
            }

            float celsius = static_cast<float>(atof(argv[2]));
            UiSetSetpointTenths(static_cast<int16_t>(celsius * 10.0f));
            cli_format::keyFloat(cli, "SETPOINT", UiSetpointTenths() / 10.0f, 1);
            return;
        }

        if (argc >= 2 && strcmp(argv[1], "press") == 0) {
            if (argc < 4) {
                cli.write("Usage: ui press <x> <y>\r\n");
                return;
            }

            int x = atoi(argv[2]);
            int y = atoi(argv[3]);
            if (x < 0 || y < 0) {
                cli.write("Usage: ui press <x> <y>\r\n");
                return;
            }

            UiZone zone = UiSyntheticPress(static_cast<uint16_t>(x), static_cast<uint16_t>(y));
            cli_format::keyText(cli, "ZONE", UiZoneName(zone));
            cli_format::keyText(cli, "EVENT", UiLastEvent());
            cli_format::keyFloat(cli, "SETPOINT", UiSetpointTenths() / 10.0f, 1);
            cli_format::keyText(cli, "HEATER", UiHeaterOn() ? "on" : "off");
            return;
        }

        if (argc >= 2 && strcmp(argv[1], "zones") == 0) {
            for (size_t i = 0; i < UiZoneCount(); ++i) {
                UiZone zone = UiZoneAt(i);
                const UiRect &r = UiZoneRect(zone);

                cli.writef("%s: %d,%d %dx%d\r\n", UiZoneName(zone), r.x, r.y, r.w, r.h);
            }
            return;
        }

        if (argc >= 2) {
            cli.write("Usage: ui [setpoint <celsius>|press <x> <y>|zones]\r\n");
            return;
        }

        cli_format::keyText(cli, "SCREEN", "home");
        cli_format::keyFloat(cli, "SETPOINT", UiSetpointTenths() / 10.0f, 1);
        cli_format::keyText(cli, "HEATER", UiHeaterOn() ? "on" : "off");
        cli_format::keyText(cli, "PRESSED", UiZoneName(UiPressedZone()));
        cli_format::keyText(cli, "LAST_EVENT", UiLastEvent());
    }
}

namespace ui_commands {
    void registerCommands(cli::SerialCli &cli) {
        cli.registerCommand({"ui", [&cli](int argc, char **argv) { cmdUi(cli, argc, argv); }, "touch UI: state, ui setpoint <celsius>, ui press <x> <y>, ui zones"});
    }
}
