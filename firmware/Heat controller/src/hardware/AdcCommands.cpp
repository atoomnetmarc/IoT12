/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "AdcCommands.h"

#include <cstdio>
#include <cstring>

#include "AdcExternal.h"
#include "CliFormat.h"
#include "config.h"

namespace {
    struct Channel {
        const char *argument;
        const char *key;
        uint8_t index;
    };

    const Channel channels[] = {
        {"temp", "TEMP", AIN_EXTERNAL_ADC_TEMP},
        {"ntc", "NTC", AIN_EXTERNAL_ADC_NTC},
        {"current", "IRON_CURRENT", AIN_EXTERNAL_ADC_IRON_CURRENT},
        {"vin", "VIN", AIN_EXTERNAL_ADC_VIN},
    };

    void report(cli::SerialCli &cli, const Channel &channel) {
        int16_t raw = 0;
        float volts = 0.0f;

        ADCExternal.readChannelRaw(channel.index, raw, volts);

        char key[32];

        snprintf(key, sizeof(key), "ADC_%s_RAW", channel.key);
        cli_format::keyInt(cli, key, raw);

        snprintf(key, sizeof(key), "ADC_%s_V", channel.key);
        cli_format::keyFloat(cli, key, volts, 5);
    }

    void cmdAdc(cli::SerialCli &cli, int argc, char **argv) {
        if (argc == 1) {
            for (const Channel &channel : channels) {
                report(cli, channel);
            }
            return;
        }

        for (const Channel &channel : channels) {
            if (strcmp(argv[1], channel.argument) == 0) {
                report(cli, channel);
                return;
            }
        }

        cli.write("Usage: adc [temp|ntc|current|vin]\r\n");
    }
}

namespace adc_commands {
    void registerCommands(cli::SerialCli &cli) {
        cli.registerCommand({"adc", [&cli](int argc, char **argv) { cmdAdc(cli, argc, argv); }, "raw and converted ADS1115 channels (adc [temp|ntc|current|vin])"});
    }
}
