/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "SystemCommands.h"

#include <Arduino.h>
#include <esp_system.h>

#include <cstring>

#include "CliFormat.h"
#include "EventLog.h"

#ifndef PRODUCT_VERSION
#define PRODUCT_VERSION "unknown"
#endif

namespace {
    bool _rebootPending = false;

    const char *resetReasonName(esp_reset_reason_t reason) {
        switch (reason) {
            case ESP_RST_POWERON:
                return "POWERON";
            case ESP_RST_EXT:
                return "EXT";
            case ESP_RST_SW:
                return "SW";
            case ESP_RST_PANIC:
                return "PANIC";
            case ESP_RST_INT_WDT:
                return "INT_WDT";
            case ESP_RST_TASK_WDT:
                return "TASK_WDT";
            case ESP_RST_WDT:
                return "WDT";
            case ESP_RST_DEEPSLEEP:
                return "DEEPSLEEP";
            case ESP_RST_BROWNOUT:
                return "BROWNOUT";
            case ESP_RST_SDIO:
                return "SDIO";
            default:
                return "UNKNOWN";
        }
    }

    void cmdVersion(cli::SerialCli &cli, int argc, char **argv) {
        (void)argc;
        (void)argv;

        cli_format::keyText(cli, "VERSION", PRODUCT_VERSION);
        cli_format::keyText(cli, "BUILD", __DATE__ " " __TIME__);
        cli_format::keyText(cli, "CHIP", ESP.getChipModel());
        cli_format::keyUInt(cli, "CORES", ESP.getChipCores());
        cli_format::keyUInt(cli, "CPU_MHZ", ESP.getCpuFreqMHz());
        cli_format::keyUInt(cli, "FLASH_MB", ESP.getFlashChipSize() / 1024U / 1024U);
        cli_format::keyText(cli, "RESET", resetReasonName(esp_reset_reason()));
    }

    void cmdUptime(cli::SerialCli &cli, int argc, char **argv) {
        (void)argc;
        (void)argv;

        uint32_t ms = millis();
        uint32_t seconds = ms / 1000U;

        cli_format::keyUInt(cli, "UPTIME_MS", ms);
        cli.writef("UPTIME: %lud %02lu:%02lu:%02lu\r\n",
                   static_cast<unsigned long>(seconds / 86400U),
                   static_cast<unsigned long>((seconds / 3600U) % 24U),
                   static_cast<unsigned long>((seconds / 60U) % 60U),
                   static_cast<unsigned long>(seconds % 60U));
    }

    void cmdMem(cli::SerialCli &cli, int argc, char **argv) {
        (void)argc;
        (void)argv;

        cli_format::keyUInt(cli, "FREE_HEAP", ESP.getFreeHeap());
        cli_format::keyUInt(cli, "MIN_FREE_HEAP", ESP.getMinFreeHeap());
        cli_format::keyUInt(cli, "MAX_ALLOC", ESP.getMaxAllocHeap());
        cli_format::keyUInt(cli, "HEAP_SIZE", ESP.getHeapSize());
    }

    void cmdEvents(cli::SerialCli &cli, int argc, char **argv) {
        if (argc >= 2 && strcmp(argv[1], "clear") == 0) {
            eventlog::Log.clear();
            cli.write("EVENTS: cleared\r\n");
            return;
        }

        if (argc >= 2 && strcmp(argv[1], "live") == 0) {
            if (argc < 3) {
                cli.writef("EVENTS_LIVE: %s\r\n", eventlog::Log.live() ? "on" : "off");
                return;
            }

            if (strcmp(argv[2], "on") == 0) {
                eventlog::Log.setLive(true);
            } else if (strcmp(argv[2], "off") == 0) {
                eventlog::Log.setLive(false);
            } else {
                cli.write("Usage: events live on|off\r\n");
                return;
            }

            cli.writef("EVENTS_LIVE: %s\r\n", eventlog::Log.live() ? "on" : "off");
            return;
        }

        if (argc >= 2) {
            cli.write("Usage: events [clear|live on|off]\r\n");
            return;
        }

        cli.writef("EVENTS: %u\r\n", static_cast<unsigned>(eventlog::Log.count()));

        for (size_t i = 0; i < eventlog::Log.count(); ++i) {
            const eventlog::Record &entry = eventlog::Log.at(i);
            cli.writef("[%10lu] %-16s %ld\r\n",
                       static_cast<unsigned long>(entry.ms),
                       eventlog::EventLog::name(entry.code),
                       static_cast<long>(entry.value));
        }
    }

    void cmdReboot(cli::SerialCli &cli, int argc, char **argv) {
        if (argc < 2 || strcmp(argv[1], "yes-really") != 0) {
            cli.write("Usage: reboot yes-really\r\n");
            return;
        }

        cli.write("REBOOT: now\r\n");
        _rebootPending = true;
    }
}

namespace system_commands {
    void registerCommands(cli::SerialCli &cli) {
        cli.registerCommand({"version", [&cli](int argc, char **argv) { cmdVersion(cli, argc, argv); }, "firmware version, build, chip, reset reason"});
        cli.registerCommand({"uptime", [&cli](int argc, char **argv) { cmdUptime(cli, argc, argv); }, "time since boot"});
        cli.registerCommand({"mem", [&cli](int argc, char **argv) { cmdMem(cli, argc, argv); }, "heap: free, minimum free, largest block"});
        cli.registerCommand({"events", [&cli](int argc, char **argv) { cmdEvents(cli, argc, argv); }, "dump events; events clear; events live on|off"});
        cli.registerCommand({"reboot", [&cli](int argc, char **argv) { cmdReboot(cli, argc, argv); }, "restart the controller (reboot yes-really)"});
    }

    bool rebootPending(void) {
        return _rebootPending;
    }
}
