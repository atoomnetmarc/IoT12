/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "HeaterCommands.h"

#include <cstdlib>
#include <cstring>

#include "CliFormat.h"
#include "Heater.h"
#include "Setting.h"

namespace {
    void cmdSensor(cli::SerialCli &cli, int argc, char **argv) {
        if (argc >= 2 && strcmp(argv[1], "offset") == 0) {
            if (argc >= 3) {
                Setting.thermocoupleInputOffsetVoltage = atof(argv[2]);
                Setting.save();
            }

            cli_format::keyFloat(cli, "THERMOCOUPLE_OFFSET_V", Setting.thermocoupleInputOffsetVoltage, 6);
            return;
        }

        HeaterStatus status = Heater.status();

        cli_format::keyFloat(cli, "TIP_TEMP_C", status.tTipC, 2);
        cli_format::keyFloat(cli, "NTC_TEMP_C", status.tAmbientC, 2);
        cli_format::keyFloat(cli, "NTC_V", status.vNTC, 5);
        cli_format::keyFloat(cli, "THERMOCOUPLE_MV", status.vThermocouple * 1000.0f, 3);
        cli_format::keyFloat(cli, "VIN_OFF_V", status.vVINHeatOff, 2);
        cli_format::keyFloat(cli, "VIN_ON_V", status.vVINHeatOn, 2);
        cli_format::keyFloat(cli, "IRON_CURRENT_OFF_A", status.iIronPowerOff, 3);
        cli_format::keyFloat(cli, "IRON_CURRENT_ON_A", status.iIronPowerOn, 3);
        cli_format::keyFloat(cli, "RTIP_OHM", status.rTip, 2);
        cli_format::keyFloat(cli, "RSUPPLY_OHM", status.rSupply, 2);
        cli_format::keyUInt(cli, "ADC_READ_MS", status.adcReadMs);
        cli_format::keyUInt(cli, "CYCLE_COUNT", status.cycleCount);
        cli_format::keyUInt(cli, "LAST_CYCLE_MS", status.lastCycleMs);
    }

    void cmdHeater(cli::SerialCli &cli, int argc, char **argv) {
        (void)argc;
        (void)argv;

        HeaterStatus status = Heater.status();

        cli_format::keyText(cli, "STATE", Setting.heaterState == HEATER_STATE::ON ? "ON" : "OFF");
        cli_format::keyFloat(cli, "SETPOINT_C", Setting.targetTemperatureHeater - 273.15f, 2);
        cli_format::keyFloat(cli, "TARGET_C", status.tTargetC, 2);
        cli_format::keyFloat(cli, "HEATFACTOR", status.heatfactor, 5);
        cli_format::keyUInt(cli, "PWM_DUTY", status.pwmDuty);
        cli_format::keyUInt(cli, "PWM_PERIOD", status.pwmPeriod);
        cli_format::keyFloat(cli, "DUTY_PCT", status.pwmPeriod != 0 ? (100.0f * static_cast<float>(status.pwmDuty)) / static_cast<float>(status.pwmPeriod) : 0.0f, 2);
    }

    void cmdPid(cli::SerialCli &cli, int argc, char **argv) {
        (void)argc;
        (void)argv;

        HeaterStatus status = Heater.status();

        cli_format::keyFloat(cli, "KP", status.kp, 5);
        cli_format::keyFloat(cli, "KI", status.ki, 5);
        cli_format::keyFloat(cli, "KD", status.kd, 5);
        cli_format::keyFloat(cli, "PTERM", status.pTerm, 5);
        cli_format::keyFloat(cli, "ITERM", status.iTerm, 5);
        cli_format::keyFloat(cli, "DTERM", status.dTerm, 5);
        cli_format::keyFloat(cli, "HEATFACTOR", status.heatfactor, 5);
    }
}

namespace heater_commands {
    void registerCommands(cli::SerialCli &cli) {
        cli.registerCommand({"sensor", [&cli](int argc, char **argv) { cmdSensor(cli, argc, argv); }, "tip and NTC temperature, VIN, iron current, Rtip, Rsupply; sensor offset [<volts>]"});
        cli.registerCommand({"heater", [&cli](int argc, char **argv) { cmdHeater(cli, argc, argv); }, "state, setpoint, heatfactor, PWM duty"});
        cli.registerCommand({"pid", [&cli](int argc, char **argv) { cmdPid(cli, argc, argv); }, "PID gains and terms"});
    }
}
