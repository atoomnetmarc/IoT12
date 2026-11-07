/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "Temperature.h"

#include <cstring>

namespace temperature {
    namespace {
        // K = C + 273.15
        // Source: https://en.wikipedia.org/wiki/Celsius
        float celsiusToKelvin(float value) {
            return value + 273.15f;
        }

        float celsiusFromKelvin(float kelvin) {
            return kelvin - 273.15f;
        }

        // K = (F - 32) * 5/9 + 273.15
        // Source: https://en.wikipedia.org/wiki/Fahrenheit
        float fahrenheitToKelvin(float value) {
            return (value - 32.0f) * 5.0f / 9.0f + 273.15f;
        }

        float fahrenheitFromKelvin(float kelvin) {
            return (kelvin - 273.15f) * 9.0f / 5.0f + 32.0f;
        }

        // K = K
        // Source: https://en.wikipedia.org/wiki/Kelvin
        float kelvinToKelvin(float value) {
            return value;
        }

        float kelvinFromKelvin(float kelvin) {
            return kelvin;
        }

        // K = Ra * 5/9
        // Source: https://en.wikipedia.org/wiki/Rankine_scale
        float rankineToKelvin(float value) {
            return value * 5.0f / 9.0f;
        }

        float rankineFromKelvin(float kelvin) {
            return kelvin * 9.0f / 5.0f;
        }

        // K = Re * 5/4 + 273.15
        // Source: https://en.wikipedia.org/wiki/R%C3%A9aumur_scale
        float reaumurToKelvin(float value) {
            return value * 5.0f / 4.0f + 273.15f;
        }

        float reaumurFromKelvin(float kelvin) {
            return (kelvin - 273.15f) * 4.0f / 5.0f;
        }

        // K = (Ro - 7.5) * 40/21 + 273.15
        // Source: https://en.wikipedia.org/wiki/R%C3%B8mer_scale
        float romerToKelvin(float value) {
            return (value - 7.5f) * 40.0f / 21.0f + 273.15f;
        }

        float romerFromKelvin(float kelvin) {
            return (kelvin - 273.15f) * 21.0f / 40.0f + 7.5f;
        }

        // K = 373.15 - De * 2/3
        // Source: https://en.wikipedia.org/wiki/Delisle_scale
        float delisleToKelvin(float value) {
            return 373.15f - value * 2.0f / 3.0f;
        }

        float delisleFromKelvin(float kelvin) {
            return (373.15f - kelvin) * 3.0f / 2.0f;
        }

        // K = N * 100/33 + 273.15
        // Source: https://en.wikipedia.org/wiki/Newton_scale
        float newtonToKelvin(float value) {
            return value * 100.0f / 33.0f + 273.15f;
        }

        float newtonFromKelvin(float kelvin) {
            return (kelvin - 273.15f) * 33.0f / 100.0f;
        }

        // K = L + 20.15
        // 0 L is the boiling point of hydrogen (-253 C) and 70 L the boiling
        // point of oxygen (-183 C).
        // Source: https://en.wikipedia.org/wiki/Leiden_scale
        float leidenToKelvin(float value) {
            return value + 20.15f;
        }

        float leidenFromKelvin(float kelvin) {
            return kelvin - 20.15f;
        }

        // K = 580.8 + 273.15 + W * 72
        // 0 W is 1077.5 F (580.8 C) and each step is 130 F (72 C).
        // Source: https://en.wikipedia.org/wiki/Wedgwood_scale
        float wedgwoodToKelvin(float value) {
            return 853.95f + value * 72.0f;
        }

        float wedgwoodFromKelvin(float kelvin) {
            return (kelvin - 853.95f) / 72.0f;
        }

        // Gas mark is a discrete oven dial. The conversion table is the
        // practical definition; values between marks are linearly interpolated.
        // Source: https://en.wikipedia.org/wiki/Gas_mark
        const float GAS_MARK[] = {0.25f, 0.5f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
        const float GAS_MARK_CELSIUS[] = {110.0f, 120.0f, 140.0f, 150.0f, 170.0f, 180.0f, 190.0f, 200.0f, 220.0f, 230.0f, 240.0f, 260.0f};
        const size_t GAS_MARK_COUNT = sizeof(GAS_MARK) / sizeof(GAS_MARK[0]);

        // Returns the index of the table segment that contains the value.
        size_t gasMarkSegment(const float *table, float value) {
            size_t index = 0;
            while (index + 2 < GAS_MARK_COUNT && value > table[index + 1]) {
                index++;
            }
            return index;
        }

        float gasMarkToKelvin(float value) {
            size_t index = gasMarkSegment(GAS_MARK, value);
            float slope = (GAS_MARK_CELSIUS[index + 1] - GAS_MARK_CELSIUS[index]) / (GAS_MARK[index + 1] - GAS_MARK[index]);
            return GAS_MARK_CELSIUS[index] + (value - GAS_MARK[index]) * slope + 273.15f;
        }

        float gasMarkFromKelvin(float kelvin) {
            float celsius = kelvin - 273.15f;
            size_t index = gasMarkSegment(GAS_MARK_CELSIUS, celsius);
            float slope = (GAS_MARK[index + 1] - GAS_MARK[index]) / (GAS_MARK_CELSIUS[index + 1] - GAS_MARK_CELSIUS[index]);
            return GAS_MARK[index] + (celsius - GAS_MARK_CELSIUS[index]) * slope;
        }

        // K = Th * 30 + 273.15
        // Thermostat 1 is 30 C and each whole number adds 30 C.
        // Source: https://en.wikipedia.org/wiki/Gas_mark
        float thermostatToKelvin(float value) {
            return value * 30.0f + 273.15f;
        }

        float thermostatFromKelvin(float kelvin) {
            return (kelvin - 273.15f) / 30.0f;
        }

        // K = St * 20 + 393.15
        // Stufe 1 is 140 C and each step adds 20 C.
        // Source: https://en.wikipedia.org/wiki/Gas_mark
        float stufeToKelvin(float value) {
            return value * 20.0f + 393.15f;
        }

        float stufeFromKelvin(float kelvin) {
            return (kelvin - 393.15f) / 20.0f;
        }

        const Unit UNITS[] = {
            {"celsius", "Celsius", "°C", celsiusToKelvin, celsiusFromKelvin, 10.0f, 1, 1, true},
            {"fahrenheit", "Fahrenheit", "°F", fahrenheitToKelvin, fahrenheitFromKelvin, 18.0f, 1, 1, true},
            {"kelvin", "Kelvin", "K", kelvinToKelvin, kelvinFromKelvin, 10.0f, 1, 1, true},
            {"rankine", "Rankine", "°Ra", rankineToKelvin, rankineFromKelvin, 18.0f, 1, 1, false},
            {"reaumur", "Réaumur", "°Ré", reaumurToKelvin, reaumurFromKelvin, 8.0f, 1, 1, false},
            {"romer", "Rømer", "°Rø", romerToKelvin, romerFromKelvin, 5.25f, 1, 1, false},
            {"delisle", "Delisle", "°De", delisleToKelvin, delisleFromKelvin, 15.0f, -1, 1, false},
            {"newton", "Newton", "°N", newtonToKelvin, newtonFromKelvin, 3.3f, 1, 1, false},
            {"leiden", "Leiden", "°L", leidenToKelvin, leidenFromKelvin, 10.0f, 1, 1, false},
            {"wedgwood", "Wedgwood", "°W", wedgwoodToKelvin, wedgwoodFromKelvin, 0.14f, 1, 2, false},
            {"gasmark", "Gas mark", "GM", gasMarkToKelvin, gasMarkFromKelvin, 0.5f, 1, 2, false},
            {"thermostat", "Thermostat", "Th", thermostatToKelvin, thermostatFromKelvin, 0.33f, 1, 1, false},
            {"stufe", "Stufe", "St", stufeToKelvin, stufeFromKelvin, 0.5f, 1, 1, false},
        };
    }

    const Unit *units(void) {
        return UNITS;
    }

    size_t unitCount(void) {
        return sizeof(UNITS) / sizeof(UNITS[0]);
    }

    const Unit *find(const char *id) {
        if (id == nullptr) {
            return nullptr;
        }

        for (const Unit &unit : UNITS) {
            if (strcmp(unit.id, id) == 0) {
                return &unit;
            }
        }

        return nullptr;
    }

    const Unit *celsius(void) {
        return &UNITS[0];
    }
}
