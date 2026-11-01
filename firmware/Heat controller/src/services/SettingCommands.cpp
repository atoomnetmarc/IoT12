/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "SettingCommands.h"

#include "CliFormat.h"
#include "Setting.h"

namespace {
    void cmdSettings(cli::SerialCli &cli, int argc, char **argv) {
        (void)argc;
        (void)argv;

        cli_format::keyText(cli, "HEATER_STATE", Setting.heaterState == HEATER_STATE::ON ? "ON" : "OFF");
        cli_format::keyFloat(cli, "TARGET_C", Setting.targetTemperatureHeater - 273.15f, 2);
        cli_format::keyFloat(cli, "GAIN_VSUPPLY", Setting.gainVSupplyResistorDivider, 5);
        cli_format::keyFloat(cli, "GAIN_HEATER_TEMP", Setting.gainHeaterTemperatureAmplifier, 3);
        cli_format::keyFloat(cli, "GAIN_HEATER_CURRENT", Setting.gainHeaterCurrentAmplifier, 3);
        cli_format::keyFloat(cli, "SHUNT_OHM", Setting.resistanceHeaterShunt, 5);
        cli_format::keyFloat(cli, "NTC_PULLUP_OHM", Setting.resistanceNTCPullUp, 0);
        cli_format::keyFloat(cli, "THERMOCOUPLE_OFFSET_V", Setting.thermocoupleInputOffsetVoltage, 6);
    }
}

namespace setting_commands {
    void registerCommands(cli::SerialCli &cli) {
        cli.registerCommand({"settings", [&cli](int argc, char **argv) { cmdSettings(cli, argc, argv); }, "current settings values"});
    }
}
