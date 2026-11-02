/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "ShakeCommands.h"

#include <cstdlib>
#include <cstring>

#include "AdcInternal.h"
#include "CliFormat.h"
#include "Setting.h"
#include "Shake.h"

namespace {
    const char *phaseName(SHAKE_PHASE phase) {
        switch (phase) {
            case SHAKE_PHASE::ACTIVE:
                return "ACTIVE";
            case SHAKE_PHASE::RAMP:
                return "RAMP";
            case SHAKE_PHASE::STANDBY2:
                return "STANDBY2";
            case SHAKE_PHASE::EXPIRED:
                return "EXPIRED";
        }

        return "UNKNOWN";
    }

    void cmdShake(cli::SerialCli &cli, int argc, char **argv) {
        if (argc >= 2 && strcmp(argv[1], "adc") == 0) {
            cli_format::keyFloat(cli, "SHAKE_A3_V", ADCInternal.readChannelVdd(A3), 4);
            cli_format::keyFloat(cli, "SHAKE_A2_V", ADCInternal.readChannelVdd(A2), 4);
            return;
        }

        if (argc >= 2 && strcmp(argv[1], "standby") == 0) {
            if (argc >= 3) {
                Setting.standbyTemperatureHeater = static_cast<float>(atof(argv[2])) + 273.15f;
                Setting.save();
            }

            cli_format::keyFloat(cli, "STANDBY_C", Setting.standbyTemperatureHeater - 273.15f, 2);
            return;
        }

        if (argc >= 2 && strcmp(argv[1], "standby2") == 0) {
            if (argc >= 3) {
                Setting.standbyTemperatureHeater2 = static_cast<float>(atof(argv[2])) + 273.15f;
                Setting.save();
            }

            cli_format::keyFloat(cli, "STANDBY2_C", Setting.standbyTemperatureHeater2 - 273.15f, 2);
            return;
        }

        if (argc >= 2 && strcmp(argv[1], "timeout") == 0) {
            if (argc >= 3) {
                Setting.shakeStage1Ms = static_cast<float>(strtoul(argv[2], nullptr, 0));
                Setting.save();
            }

            cli_format::keyUInt(cli, "STAGE1_MS", static_cast<unsigned long>(Setting.shakeStage1Ms));
            return;
        }

        if (argc >= 2 && strcmp(argv[1], "stage2") == 0) {
            if (argc >= 3) {
                Setting.shakeStage2Ms = static_cast<float>(strtoul(argv[2], nullptr, 0));
                Setting.save();
            }

            cli_format::keyUInt(cli, "STAGE2_MS", static_cast<unsigned long>(Setting.shakeStage2Ms));
            return;
        }

        if (argc >= 2 && strcmp(argv[1], "off") == 0) {
            if (argc >= 3) {
                Setting.shakeOffMs = static_cast<float>(strtoul(argv[2], nullptr, 0));
                Setting.save();
            }

            cli_format::keyUInt(cli, "OFF_MS", static_cast<unsigned long>(Setting.shakeOffMs));
            return;
        }

        if (argc >= 2) {
            cli.write("Usage: shake [adc|standby [<celsius>]|standby2 [<celsius>]|timeout [<ms>]|stage2 [<ms>]|off [<ms>]]\r\n");
            return;
        }

        cli_format::keyText(cli, "RAW", Shake.rawLevel() ? "HIGH" : "LOW");
        cli_format::keyText(cli, "DEBOUNCED", Shake.debouncedLevel() ? "HIGH" : "LOW");
        cli_format::keyFloat(cli, "SHAKE_V", Shake.voltage(), 4);
        cli_format::keyUInt(cli, "LAST_MOTION_MS", Shake.lastMotionAgeMs());
        cli_format::keyText(cli, "PHASE", phaseName(Shake.phase()));
        cli_format::keyText(cli, "STANDBY", Shake.standbyActive() ? "yes" : "no");
        cli_format::keyText(cli, "EXPIRED", Shake.expired() ? "yes" : "no");
        cli_format::keyFloat(cli, "TARGET_C", Shake.targetTemperature() - 273.15f, 2);
        cli_format::keyFloat(cli, "STANDBY_C", Setting.standbyTemperatureHeater - 273.15f, 2);
        cli_format::keyFloat(cli, "STANDBY2_C", Setting.standbyTemperatureHeater2 - 273.15f, 2);
        cli_format::keyUInt(cli, "STAGE1_MS", static_cast<unsigned long>(Setting.shakeStage1Ms));
        cli_format::keyUInt(cli, "STAGE2_MS", static_cast<unsigned long>(Setting.shakeStage2Ms));
        cli_format::keyUInt(cli, "OFF_MS", static_cast<unsigned long>(Setting.shakeOffMs));
    }
}

namespace shake_commands {
    void registerCommands(cli::SerialCli &cli) {
        cli.registerCommand({"shake", [&cli](int argc, char **argv) { cmdShake(cli, argc, argv); }, "SHAKE input and standby phases; shake adc; shake standby|standby2 [<celsius>]; shake timeout|stage2|off [<ms>]"});
    }
}
