/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "DisplayCommands.h"

#include <cstdlib>
#include <cstring>

#include "CliFormat.h"
#include "TouchCalibration.h"
#include "TouchTest.h"
#include "display.h"

namespace {
    void cmdDisplay(cli::SerialCli &cli, int argc, char **argv) {
        if (argc >= 2 && strcmp(argv[1], "backlight") == 0) {
            if (argc < 3) {
                cli.write("Usage: display backlight <0-255>\r\n");
                return;
            }

            int level = atoi(argv[2]);
            if (level < 0 || level > 255) {
                cli.write("Usage: display backlight <0-255>\r\n");
                return;
            }

            DisplaySetBacklight(static_cast<uint8_t>(level));
            cli_format::keyUInt(cli, "BACKLIGHT", DisplayBacklight());
            return;
        }

        if (argc >= 2 && strcmp(argv[1], "on") == 0) {
            DisplaySetPower(true);
            cli_format::keyText(cli, "POWER", "on");
            return;
        }

        if (argc >= 2 && strcmp(argv[1], "off") == 0) {
            DisplaySetPower(false);
            cli_format::keyText(cli, "POWER", "off");
            return;
        }

        if (argc >= 2 && strcmp(argv[1], "box") == 0) {
            if (argc >= 3 && strcmp(argv[2], "off") == 0) {
                DisplayBoxStop();
                cli_format::keyText(cli, "BOX", "off");
                return;
            }

            if (argc < 3) {
                cli.write("Usage: display box <margin 0-119>|off\r\n");
                return;
            }

            int margin = atoi(argv[2]);
            if (margin < 0 || margin > 119) {
                cli.write("Usage: display box <margin 0-119>|off\r\n");
                return;
            }

            DisplayDrawBox(static_cast<uint16_t>(margin));
            cli_format::keyUInt(cli, "BOX_MARGIN", static_cast<uint32_t>(margin));
            return;
        }

        if (argc >= 2) {
            cli.write("Usage: display [backlight <0-255>|on|off|box <margin>|off]\r\n");
            return;
        }

        cli_format::keyText(cli, "DRIVER", DisplayDriverName());
        cli.writef("RESOLUTION: %ux%u\r\n", DisplayWidth(), DisplayHeight());
        cli_format::keyUInt(cli, "ROTATION", DisplayRotation());
        cli_format::keyUInt(cli, "BACKLIGHT", DisplayBacklight());
        cli_format::keyUInt(cli, "LAST_DRAW_MS", DisplayLastDrawMs());
    }

    void cmdTouch(cli::SerialCli &cli, int argc, char **argv) {
        if (argc >= 2 && strcmp(argv[1], "calibrate") == 0) {
            if (argc >= 3 && strcmp(argv[2], "abort") == 0) {
                touch_calibration::abort();
                cli_format::keyText(cli, "CALIBRATION", "aborted");
                return;
            }

            if (argc >= 3 && strcmp(argv[2], "reset") == 0) {
                touch_calibration::reset();
                cli_format::keyText(cli, "CALIBRATION", "reset");
                return;
            }

            bool started = touch_calibration::start();
            cli_format::keyText(cli, "CALIBRATION", started ? "started" : "busy");
            return;
        }

        if (argc >= 2 && strcmp(argv[1], "test") == 0) {
            if (argc >= 3 && strcmp(argv[2], "stop") == 0) {
                touch_test::stop();
                cli_format::keyText(cli, "TEST", "stopped");
                return;
            }

            bool started = touch_test::start();
            cli_format::keyText(cli, "TEST", started ? "started" : "busy");
            return;
        }

        uint16_t x = 0;
        uint16_t y = 0;
        uint16_t rawX = 0;
        uint16_t rawY = 0;
        bool pressed = TouchRead(&x, &y, &rawX, &rawY);

        cli_format::keyText(cli, "IRQ", TouchIrqLevel() ? "high" : "low");
        cli_format::keyText(cli, "PRESSED", pressed ? "yes" : "no");
        cli_format::keyUInt(cli, "RAW_X", rawX);
        cli_format::keyUInt(cli, "RAW_Y", rawY);
        cli_format::keyUInt(cli, "X", x);
        cli_format::keyUInt(cli, "Y", y);
        cli_format::keyText(cli, "STORED", touch_calibration::stored() ? "yes" : "no");
        cli_format::keyText(cli, "STEP", touch_calibration::stepName());
        cli_format::keyText(cli, "TEST", touch_test::active() ? "yes" : "no");

        for (size_t i = 0; i < touch_calibration::valueCount(); i += 2) {
            cli.writef("P%u: %u,%u\r\n", static_cast<unsigned>(i / 2), touch_calibration::valueAt(i), touch_calibration::valueAt(i + 1));
        }
    }
}

namespace display_commands {
    void registerCommands(cli::SerialCli &cli) {
        cli.registerCommand({"display", [&cli](int argc, char **argv) { cmdDisplay(cli, argc, argv); }, "display driver, resolution, backlight; display backlight <0-255>|on|off|box <margin>|off"});
        cli.registerCommand({"touch", [&cli](int argc, char **argv) { cmdTouch(cli, argc, argv); }, "touch panel: state, raw and mapped coordinates; touch calibrate|abort|reset|test|test stop"});
    }
}
