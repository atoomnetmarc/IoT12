/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <math.h>
#include <stddef.h>
#include <stdint.h>

#include "SerialCli.h"

/*
 * Output helpers for the `KEY: value` line format.
 *
 * Floats are formatted by hand: the SAMD21 links newlib-nano without
 * _printf_float, so "%f" in vsnprintf produces nothing. Doing it here keeps
 * both firmwares on one implementation.
 */
namespace cli_format {
    inline void formatFloat(char *buffer, size_t size, float value, uint8_t decimals) {
        if (size == 0) {
            return;
        }

        if (isnan(value)) {
            buffer[0] = 'n';
            buffer[1 < size ? 1 : 0] = 'a';
            buffer[2 < size ? 2 : 0] = 'n';
            buffer[3 < size ? 3 : 0] = '\0';
            return;
        }

        if (isinf(value)) {
            const char *text = value < 0 ? "-inf" : "inf";
            size_t i = 0;
            while (text[i] != '\0' && i + 1 < size) {
                buffer[i] = text[i];
                i++;
            }
            buffer[i] = '\0';
            return;
        }

        bool negative = value < 0.0f;
        if (negative) {
            value = -value;
        }

        /*
         * The scaled value needs 64 bits: 10000 with 6 decimals is 1e10, which
         * overflows uint32_t. The product is formed in double, because a float
         * product loses the low digits before the rounding step.
         */
        uint64_t scale = 1;
        for (uint8_t i = 0; i < decimals; ++i) {
            scale *= 10U;
        }

        uint64_t scaled = static_cast<uint64_t>(static_cast<double>(value) * static_cast<double>(scale) + 0.5);
        uint64_t whole = scaled / scale;
        uint64_t fraction = scaled % scale;

        size_t pos = 0;
        if (negative && pos + 1 < size) {
            buffer[pos++] = '-';
        }

        char digits[20];
        size_t count = 0;
        do {
            digits[count++] = static_cast<char>('0' + (whole % 10U));
            whole /= 10U;
        } while (whole > 0 && count < sizeof(digits));

        while (count > 0 && pos + 1 < size) {
            buffer[pos++] = digits[--count];
        }

        if (decimals > 0 && pos + 1 < size) {
            buffer[pos++] = '.';
            for (uint8_t i = decimals; i > 0 && pos + 1 < size; --i) {
                uint64_t divisor = 1;
                for (uint8_t j = 1; j < i; ++j) {
                    divisor *= 10U;
                }
                buffer[pos++] = static_cast<char>('0' + ((fraction / divisor) % 10U));
            }
        }

        buffer[pos] = '\0';
    }

    inline void keyFloat(cli::SerialCli &cli, const char *key, float value, uint8_t decimals) {
        char buffer[24];
        formatFloat(buffer, sizeof(buffer), value, decimals);
        cli.writef("%s: %s\r\n", key, buffer);
    }

    inline void keyInt(cli::SerialCli &cli, const char *key, long value) {
        cli.writef("%s: %ld\r\n", key, value);
    }

    inline void keyUInt(cli::SerialCli &cli, const char *key, unsigned long value) {
        cli.writef("%s: %lu\r\n", key, value);
    }

    inline void keyText(cli::SerialCli &cli, const char *key, const char *value) {
        cli.writef("%s: %s\r\n", key, value);
    }
}
