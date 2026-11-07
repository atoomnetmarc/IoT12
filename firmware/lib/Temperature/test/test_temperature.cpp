/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include <cmath>
#include <cstdio>

#include "Temperature.h"

namespace {
    int failures = 0;

    // Fails the test when the actual value is outside the tolerance.
    void expectNear(const char *label, float actual, float expected, float tolerance) {
        if (fabsf(actual - expected) > tolerance) {
            printf("FAIL %s: got %.6f, expected %.6f\n", label, actual, expected);
            failures++;
        }
    }

    // Converts a Kelvin value to the unit and back, and checks the result.
    void expectRoundTrip(const temperature::Unit *unit, float kelvin) {
        float value = unit->fromKelvin(kelvin);
        float back = unit->toKelvin(value);
        expectNear(unit->id, back, kelvin, 0.001f);
    }
}

int main(void) {
    const temperature::Unit *celsius = temperature::find("celsius");
    const temperature::Unit *fahrenheit = temperature::find("fahrenheit");
    const temperature::Unit *kelvin = temperature::find("kelvin");
    const temperature::Unit *rankine = temperature::find("rankine");
    const temperature::Unit *reaumur = temperature::find("reaumur");
    const temperature::Unit *romer = temperature::find("romer");
    const temperature::Unit *delisle = temperature::find("delisle");
    const temperature::Unit *newton = temperature::find("newton");
    const temperature::Unit *leiden = temperature::find("leiden");
    const temperature::Unit *wedgwood = temperature::find("wedgwood");
    const temperature::Unit *gasmark = temperature::find("gasmark");
    const temperature::Unit *thermostat = temperature::find("thermostat");
    const temperature::Unit *stufe = temperature::find("stufe");

    if (celsius == nullptr || fahrenheit == nullptr || kelvin == nullptr || rankine == nullptr || reaumur == nullptr || romer == nullptr || delisle == nullptr || newton == nullptr || leiden == nullptr || wedgwood == nullptr || gasmark == nullptr || thermostat == nullptr || stufe == nullptr) {
        printf("FAIL: a unit is missing\n");
        return 1;
    }

    // Reference points: 0 C, 100 C and absolute zero.
    expectNear("celsius 0 C", celsius->toKelvin(0.0f), 273.15f, 0.001f);
    expectNear("celsius 100 C", celsius->toKelvin(100.0f), 373.15f, 0.001f);
    expectNear("fahrenheit 32 F", fahrenheit->toKelvin(32.0f), 273.15f, 0.001f);
    expectNear("fahrenheit 212 F", fahrenheit->toKelvin(212.0f), 373.15f, 0.001f);
    expectNear("kelvin 273.15 K", kelvin->toKelvin(273.15f), 273.15f, 0.001f);
    expectNear("rankine 491.67 Ra", rankine->toKelvin(491.67f), 273.15f, 0.01f);
    expectNear("reaumur 0 Re", reaumur->toKelvin(0.0f), 273.15f, 0.001f);
    expectNear("reaumur 80 Re", reaumur->toKelvin(80.0f), 373.15f, 0.001f);
    expectNear("romer 7.5 Ro", romer->toKelvin(7.5f), 273.15f, 0.001f);
    expectNear("romer 60 Ro", romer->toKelvin(60.0f), 373.15f, 0.001f);
    expectNear("delisle 0 De", delisle->toKelvin(0.0f), 373.15f, 0.001f);
    expectNear("delisle 150 De", delisle->toKelvin(150.0f), 273.15f, 0.001f);
    expectNear("newton 0 N", newton->toKelvin(0.0f), 273.15f, 0.001f);
    expectNear("newton 33 N", newton->toKelvin(33.0f), 373.15f, 0.001f);
    expectNear("leiden 0 L", leiden->toKelvin(0.0f), 20.15f, 0.001f);
    expectNear("leiden 70 L", leiden->toKelvin(70.0f), 90.15f, 0.001f);
    expectNear("wedgwood 0 W", wedgwood->toKelvin(0.0f), 853.95f, 0.001f);
    expectNear("wedgwood 240 W", wedgwood->toKelvin(240.0f), 18133.95f, 0.01f);
    expectNear("gasmark 1 GM", gasmark->toKelvin(1.0f), 413.15f, 0.001f);
    expectNear("gasmark 1.5 GM", gasmark->toKelvin(1.5f), 418.15f, 0.001f);
    expectNear("gasmark 4 GM", gasmark->toKelvin(4.0f), 453.15f, 0.001f);
    expectNear("gasmark 10 GM", gasmark->toKelvin(10.0f), 533.15f, 0.001f);
    expectNear("thermostat 1 Th", thermostat->toKelvin(1.0f), 303.15f, 0.001f);
    expectNear("thermostat 9 Th", thermostat->toKelvin(9.0f), 543.15f, 0.001f);
    expectNear("stufe 1 St", stufe->toKelvin(1.0f), 413.15f, 0.001f);
    expectNear("stufe 8 St", stufe->toKelvin(8.0f), 553.15f, 0.001f);

    // Round-trip every unit at 0 C, 100 C and absolute zero.
    for (size_t i = 0; i < temperature::unitCount(); i++) {
        const temperature::Unit *unit = &temperature::units()[i];
        expectRoundTrip(unit, 273.15f);
        expectRoundTrip(unit, 373.15f);
        expectRoundTrip(unit, 0.0f);
    }

    // Delisle moves the other way.
    if (delisle->direction != -1) {
        printf("FAIL delisle direction: got %d, expected -1\n", delisle->direction);
        failures++;
    }

    if (failures != 0) {
        printf("%d failure(s)\n", failures);
        return 1;
    }

    printf("all temperature tests passed\n");
    return 0;
}
