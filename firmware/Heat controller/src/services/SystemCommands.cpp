/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "SystemCommands.h"

#include <Arduino.h>
#include <cstring>

#include "EventLog.h"

#ifndef PRODUCT_VERSION
#define PRODUCT_VERSION "unknown"
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "unknown"
#endif

extern "C" char *sbrk(int incr);

namespace {
    bool _rebootPending = false;

    const char *resetCauseName(uint8_t cause) {
        if ((cause & PM_RCAUSE_POR) != 0) {
            return "POR";
        }
        if ((cause & PM_RCAUSE_BOD12) != 0) {
            return "BOD12";
        }
        if ((cause & PM_RCAUSE_BOD33) != 0) {
            return "BOD33";
        }
        if ((cause & PM_RCAUSE_EXT) != 0) {
            return "EXT";
        }
        if ((cause & PM_RCAUSE_WDT) != 0) {
            return "WDT";
        }
        if ((cause & PM_RCAUSE_SYST) != 0) {
            return "SYST";
        }

        return "UNKNOWN";
    }

    void cmdVersion(cli::SerialCli &cli, int argc, char **argv) {
        (void)argc;
        (void)argv;

        cli.writef("VERSION: %s\r\n", PRODUCT_VERSION);
        cli.writef("BUILD: %s %s\r\n", __DATE__, __TIME__);
        cli.writef("BOARD: %s\r\n", BOARD_NAME);
        cli.writef("CHIP: SAMD21G18A\r\n");
        cli.writef("RESET: %s\r\n", resetCauseName(PM->RCAUSE.reg));
    }

    void cmdUptime(cli::SerialCli &cli, int argc, char **argv) {
        (void)argc;
        (void)argv;

        uint32_t ms = millis();
        uint32_t seconds = ms / 1000U;

        cli.writef("UPTIME_MS: %lu\r\n", static_cast<unsigned long>(ms));
        cli.writef("UPTIME: %lud %02lu:%02lu:%02lu\r\n",
                   static_cast<unsigned long>(seconds / 86400U),
                   static_cast<unsigned long>((seconds / 3600U) % 24U),
                   static_cast<unsigned long>((seconds / 60U) % 60U),
                   static_cast<unsigned long>(seconds % 60U));
    }

    void cmdMem(cli::SerialCli &cli, int argc, char **argv) {
        (void)argc;
        (void)argv;

        char marker = 0;
        uintptr_t stack = reinterpret_cast<uintptr_t>(&marker);
        uintptr_t heap = reinterpret_cast<uintptr_t>(sbrk(0));
        uintptr_t ramEnd = static_cast<uintptr_t>(RAMEND);

        cli.writef("RAM_END: 0x%08lx\r\n", static_cast<unsigned long>(ramEnd));
        cli.writef("HEAP_END: 0x%08lx\r\n", static_cast<unsigned long>(heap));
        cli.writef("STACK_PTR: 0x%08lx\r\n", static_cast<unsigned long>(stack));
        cli.writef("FREE: %lu\r\n", static_cast<unsigned long>(stack - heap));
        cli.writef("STACK_USED: %lu\r\n", static_cast<unsigned long>(ramEnd - stack));
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
        cli.registerCommand({"version", [&cli](int argc, char **argv) { cmdVersion(cli, argc, argv); }, "firmware version, build, board, reset cause"});
        cli.registerCommand({"uptime", [&cli](int argc, char **argv) { cmdUptime(cli, argc, argv); }, "time since boot"});
        cli.registerCommand({"mem", [&cli](int argc, char **argv) { cmdMem(cli, argc, argv); }, "RAM: heap end, stack pointer, free"});
        cli.registerCommand({"events", [&cli](int argc, char **argv) { cmdEvents(cli, argc, argv); }, "dump events; events clear; events live on|off"});
        cli.registerCommand({"reboot", [&cli](int argc, char **argv) { cmdReboot(cli, argc, argv); }, "reset the controller (reboot yes-really)"});
    }

    bool rebootPending(void) {
        return _rebootPending;
    }
}
