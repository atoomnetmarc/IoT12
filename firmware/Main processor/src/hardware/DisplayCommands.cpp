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
    // Print every panel, bus, touch and backlight property as KEY: value lines.
    void printDisplayInfo(cli::SerialCli &cli) {
        DisplayInfo info;
        DisplayGetInfo(&info);

        cli_format::keyText(cli, "DRIVER", info.driver);
        cli.writef("RESOLUTION: %ux%u\r\n", info.width, info.height);
        cli_format::keyUInt(cli, "ROTATION", info.rotation);
        cli_format::keyUInt(cli, "BACKLIGHT", info.backlight);
        cli_format::keyUInt(cli, "LAST_DRAW_MS", info.lastDrawMs);
        cli_format::keyUInt(cli, "COLOR_DEPTH", info.colorDepth);

        cli.writef("ID1: 0x%08lX\r\n", static_cast<unsigned long>(info.id1));
        cli.writef("ID4: 0x%08lX\r\n", static_cast<unsigned long>(info.id4));
        cli.writef("ID9: 0x%08lX\r\n", static_cast<unsigned long>(info.id9));
        cli.writef("IDC: 0x%08lX\r\n", static_cast<unsigned long>(info.idc));

        cli_format::keyUInt(cli, "SPI_HOST", static_cast<unsigned long>(info.spiHost));
        cli_format::keyUInt(cli, "SPI_MODE", info.spiMode);
        cli_format::keyUInt(cli, "FREQ_WRITE", info.freqWrite);
        cli_format::keyUInt(cli, "FREQ_READ", info.freqRead);
        cli_format::keyText(cli, "SPI_3WIRE", info.spi3wire ? "yes" : "no");
        cli_format::keyText(cli, "USE_LOCK", info.useLock ? "yes" : "no");
        cli_format::keyUInt(cli, "DMA_CHANNEL", info.dmaChannel);
        cli_format::keyInt(cli, "PIN_SCLK", info.pinSclk);
        cli_format::keyInt(cli, "PIN_MOSI", info.pinMosi);
        cli_format::keyInt(cli, "PIN_MISO", info.pinMiso);
        cli_format::keyInt(cli, "PIN_DC", info.pinDc);
        cli_format::keyInt(cli, "PIN_IO0", info.pinIo0);
        cli_format::keyInt(cli, "PIN_IO1", info.pinIo1);
        cli_format::keyInt(cli, "PIN_IO2", info.pinIo2);
        cli_format::keyInt(cli, "PIN_IO3", info.pinIo3);

        cli_format::keyInt(cli, "PANEL_CS", info.pinCs);
        cli_format::keyInt(cli, "PANEL_RST", info.pinRst);
        cli_format::keyInt(cli, "PANEL_BUSY", info.pinBusy);
        cli_format::keyUInt(cli, "MEMORY_WIDTH", info.memoryWidth);
        cli_format::keyUInt(cli, "MEMORY_HEIGHT", info.memoryHeight);
        cli_format::keyUInt(cli, "PANEL_WIDTH", info.panelWidth);
        cli_format::keyUInt(cli, "PANEL_HEIGHT", info.panelHeight);
        cli_format::keyUInt(cli, "OFFSET_X", info.offsetX);
        cli_format::keyUInt(cli, "OFFSET_Y", info.offsetY);
        cli_format::keyUInt(cli, "OFFSET_ROTATION", info.offsetRotation);
        cli_format::keyUInt(cli, "DUMMY_READ_PIXEL", info.dummyReadPixel);
        cli_format::keyUInt(cli, "DUMMY_READ_BITS", info.dummyReadBits);
        cli_format::keyUInt(cli, "END_READ_DELAY_US", info.endReadDelayUs);
        cli_format::keyText(cli, "READABLE", info.readable ? "yes" : "no");
        cli_format::keyText(cli, "INVERT", info.invert ? "yes" : "no");
        cli_format::keyText(cli, "RGB_ORDER", info.rgbOrder ? "yes" : "no");
        cli_format::keyText(cli, "DLEN_16BIT", info.dlen16bit ? "yes" : "no");
        cli_format::keyText(cli, "BUS_SHARED", info.busShared ? "yes" : "no");

        cli_format::keyUInt(cli, "TOUCH_FREQ", info.touchFreq);
        cli_format::keyUInt(cli, "TOUCH_X_MIN", info.touchXMin);
        cli_format::keyUInt(cli, "TOUCH_X_MAX", info.touchXMax);
        cli_format::keyUInt(cli, "TOUCH_Y_MIN", info.touchYMin);
        cli_format::keyUInt(cli, "TOUCH_Y_MAX", info.touchYMax);
        cli_format::keyText(cli, "TOUCH_BUS_SHARED", info.touchBusShared ? "yes" : "no");
        cli_format::keyInt(cli, "TOUCH_INT", info.touchPinInt);
        cli_format::keyInt(cli, "TOUCH_RST", info.touchPinRst);
        cli_format::keyUInt(cli, "TOUCH_OFFSET_ROTATION", info.touchOffsetRotation);
        cli_format::keyUInt(cli, "TOUCH_SPI_HOST", static_cast<unsigned long>(info.touchSpiHost));
        cli_format::keyInt(cli, "TOUCH_SCLK", info.touchPinSclk);
        cli_format::keyInt(cli, "TOUCH_MOSI", info.touchPinMosi);
        cli_format::keyInt(cli, "TOUCH_MISO", info.touchPinMiso);
        cli_format::keyInt(cli, "TOUCH_CS", info.touchPinCs);

        cli_format::keyUInt(cli, "LIGHT_FREQ", info.lightFreq);
        cli_format::keyInt(cli, "LIGHT_PIN", info.lightPinBl);
        cli_format::keyUInt(cli, "LIGHT_OFFSET", info.lightOffset);
        cli_format::keyUInt(cli, "LIGHT_PWM_CHANNEL", info.lightPwmChannel);
        cli_format::keyText(cli, "LIGHT_INVERT", info.lightInvert ? "yes" : "no");
    }

    void cmdDisplay(cli::SerialCli &cli, int argc, char **argv) {
        if (argc >= 2 && strcmp(argv[1], "info") == 0) {
            printDisplayInfo(cli);
            return;
        }

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
            cli.write("Usage: display [info|backlight <0-255>|on|off|box <margin>|off]\r\n");
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
        cli.registerCommand({"display", [&cli](int argc, char **argv) { cmdDisplay(cli, argc, argv); }, "display driver, resolution, backlight; display info|backlight <0-255>|on|off|box <margin>|off"});
        cli.registerCommand({"touch", [&cli](int argc, char **argv) { cmdTouch(cli, argc, argv); }, "touch panel: state, raw and mapped coordinates; touch calibrate|abort|reset|test|test stop"});
    }
}
