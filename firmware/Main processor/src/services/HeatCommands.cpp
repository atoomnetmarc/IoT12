/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "HeatCommands.h"

#include <cstdlib>
#include <cstring>

#include "CliFormat.h"
#include "HeatController.h"

namespace {
    void cmdHeatInt(cli::SerialCli &cli);

    void cmdHeatState(cli::SerialCli &cli, bool on) {
        if (!HeatController.isCompatible()) {
            cli.write("heat: not available (heat controller not compatible)\r\n");
            return;
        }

        HeatController.setHeaterState(on ? HEATER_STATE::ON : HEATER_STATE::OFF);
        cli_format::keyText(cli, "STATE", on ? "ON" : "OFF");
    }

    void cmdHeatSetpoint(cli::SerialCli &cli, const char *argument) {
        if (!HeatController.isCompatible()) {
            cli.write("heat: not available (heat controller not compatible)\r\n");
            return;
        }

        float celsius = static_cast<float>(atof(argument));
        HeatController.setTargetTemperature(celsius + 273.15f);
        cli_format::keyFloat(cli, "SETPOINT_C", celsius, 2);
    }

    void cmdHeat(cli::SerialCli &cli, int argc, char **argv) {
        if (argc >= 2 && strcmp(argv[1], "int") == 0) {
            cmdHeatInt(cli);
            return;
        }

        if (argc >= 2 && strcmp(argv[1], "on") == 0) {
            cmdHeatState(cli, true);
            return;
        }

        if (argc >= 2 && strcmp(argv[1], "off") == 0) {
            cmdHeatState(cli, false);
            return;
        }

        if (argc >= 3 && strcmp(argv[1], "setpoint") == 0) {
            cmdHeatSetpoint(cli, argv[2]);
            return;
        }

        if (argc >= 2) {
            cli.write("Usage: heat [int|on|off|setpoint <celsius>]\r\n");
            return;
        }

        SerialNumber_t serial = HeatController.getSerialNumber();

        cli_format::keyText(cli, "VERSION", HeatController.getFirmwareVersion());
        cli_format::keyText(cli, "COMPATIBLE", HeatController.isCompatible() ? "yes" : "no");
        cli_format::keyText(cli, "STATE", HeatController.getHeaterState() == HEATER_STATE::ON ? "ON" : "OFF");
        cli_format::keyFloat(cli, "TIP_C", HeatController.getTemperatureTip() - 273.15f, 2);
        cli_format::keyFloat(cli, "SETPOINT_C", HeatController.getTargetTemperature() - 273.15f, 2);
        cli_format::keyFloat(cli, "NTC_C", HeatController.getTemperatureNTC() - 273.15f, 2);
        cli.writef("SERIAL: %08lx %08lx %08lx %08lx\r\n",
                   static_cast<unsigned long>(serial.word0),
                   static_cast<unsigned long>(serial.word1),
                   static_cast<unsigned long>(serial.word2),
                   static_cast<unsigned long>(serial.word3));
        cli.writef("CONST1: 0x%04x\r\n", static_cast<unsigned>(HeatController.getConst1()));
        cli_format::keyFloat(cli, "CONST2", HeatController.getConst2(), 6);
        cli.writef("LAST_INT: 0x%08lx\r\n", static_cast<unsigned long>(HeatController.getLastInterrupt()));
    }

    void cmdHeatInt(cli::SerialCli &cli) {
        uint32_t flags = HeatController.getInterrupt();

        cli.writef("RAW: 0x%08lx\r\n", static_cast<unsigned long>(flags));
        cli_format::keyText(cli, "INT_REBOOT", (flags & (1UL << INT_REBOOT)) != 0 ? "1" : "0");
        cli_format::keyText(cli, "INT_MEASUREMENTS_GROUP_A", (flags & (1UL << INT_MEASUREMENTS_GROUP_A)) != 0 ? "1" : "0");
        cli_format::keyText(cli, "INT_MEASUREMENTS_GROUP_B", (flags & (1UL << INT_MEASUREMENTS_GROUP_B)) != 0 ? "1" : "0");
        cli_format::keyText(cli, "INT_LINK_TIMEOUT", (flags & (1UL << 28)) != 0 ? "1" : "0");
    }
}

namespace heat_commands {
    void registerCommands(cli::SerialCli &cli) {
        cli.registerCommand({"heat", [&cli](int argc, char **argv) { cmdHeat(cli, argc, argv); }, "heat controller: state, temperatures, serial; heat int|on|off|setpoint <c>"});
    }
}
