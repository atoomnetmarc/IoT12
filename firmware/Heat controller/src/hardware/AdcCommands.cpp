/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "AdcCommands.h"

#include <cmath>
#include <cstdio>
#include <cstring>

#include "AdcExternal.h"
#include "AdcInternal.h"
#include "CliFormat.h"
#include "config.h"

namespace {
    struct Channel {
        const char *argument;
        const char *key;
        uint8_t index;
        pin_size_t pin;
    };

    const Channel channels[] = {
        {"temp", "TEMP", AIN_EXTERNAL_ADC_TEMP, PIN_INTERNAL_ADC_TEMP},
        {"ntc", "NTC", AIN_EXTERNAL_ADC_NTC, PIN_INTERNAL_ADC_NTC},
        {"current", "IRON_CURRENT", AIN_EXTERNAL_ADC_IRON_CURRENT, PIN_INTERNAL_ADC_IRON_CURRENT},
        {"vin", "VIN", AIN_EXTERNAL_ADC_VIN, PIN_INTERNAL_ADC_VIN},
    };

    // Reports one channel: the ADS1115 raw value, then the ADS1115 and SAMD21 voltages with their absolute difference.
    void report(cli::SerialCli &cli, const Channel &channel, bool rawOnly) {
        int16_t raw = 0;
        float adsVolts = 0.0f;

        ADCExternal.readChannelRaw(channel.index, raw, adsVolts);

        char key[32];

        snprintf(key, sizeof(key), "ADC_%s_RAW", channel.key);
        cli_format::keyInt(cli, key, raw);

        if (rawOnly) {
            return;
        }

        float internalVolts = ADCInternal.readChannel(channel.pin);

        char adsBuffer[24];
        char internalBuffer[24];
        char deltaBuffer[24];

        cli_format::formatFloat(adsBuffer, sizeof(adsBuffer), adsVolts, 5);
        cli_format::formatFloat(internalBuffer, sizeof(internalBuffer), internalVolts, 5);

        cli_format::formatFloat(deltaBuffer, sizeof(deltaBuffer), std::fabs(internalVolts - adsVolts), 5);

        snprintf(key, sizeof(key), "ADC_%s_V", channel.key);
        cli.writef("%s: %s  INTERNAL_V: %s  DELTA_V: %s\r\n", key, adsBuffer, internalBuffer, deltaBuffer);
    }

    void cmdAdc(cli::SerialCli &cli, int argc, char **argv) {
        if (argc == 1) {
            for (const Channel &channel : channels) {
                report(cli, channel, false);
            }
            return;
        }

        if (strcmp(argv[1], "raw") == 0) {
            for (const Channel &channel : channels) {
                report(cli, channel, true);
            }
            return;
        }

        for (const Channel &channel : channels) {
            if (strcmp(argv[1], channel.argument) == 0) {
                report(cli, channel, false);
                return;
            }
        }

        cli.write("Usage: adc [raw|temp|ntc|current|vin]\r\n");
    }
}

namespace adc_commands {
    void registerCommands(cli::SerialCli &cli) {
        cli.registerCommand({"adc", [&cli](int argc, char **argv) { cmdAdc(cli, argc, argv); }, "raw and converted ADS1115 channels with the SAMD21 ADC delta (adc [raw|temp|ntc|current|vin])"});
    }
}
