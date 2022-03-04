/*

Copyright 2021-2022 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#include <math.h>

#include "NTC.h"

/*

Class which is able to calculate the temperature of a NTC given the voltage of a resistor divider.

Simple schematic:
resistorDividerVoltage---[resistorDividerResistance]---[NTC]---GND

*/

NTC::NTC(void)
{
}

/**
    @param resistorDividerVoltage Voltage of the resistor divider.
    @param resistorDividerResistance The fixed resistance of the resistor divider.
    @param thermistorResistance Resistance of the NTC at thermistorTemperature.
    @param thermistorTemperature Temperature in Kelvin of the NTC at thermistorResistance.
    @param thermistorBValue β-value of the NTC in Kelvin.
*/
void NTC::SetParameters(
    float resistorDividerVoltage,
    float resistorDividerResistance,
    float thermistorResistance,
    float thermistorTemperature,
    float thermistorBValue)
{
    this->resistorDividerVoltage = resistorDividerVoltage;
    this->resistorDividerResistance = resistorDividerResistance;
    this->thermistorResistance = thermistorResistance;
    this->thermistorTemperature = thermistorTemperature;
    this->thermistorBValue = thermistorBValue;
}

/**
  @return Temperature in Kelvin
*/
float NTC::GetTemperature(float voltage)
{
    //Current though NTC.
    float I = (this->resistorDividerVoltage - voltage) / this->resistorDividerResistance;

    //NTC resistance.
    float Rntc = (voltage / I);

    //Apply Steinhart–Hart equation: https://en.wikipedia.org/wiki/Thermistor#B_or_%CE%B2_parameter_equation
    float temperature = (1 / ((log(Rntc / this->resistorDividerResistance) / this->thermistorBValue) + (1 / this->thermistorTemperature)));

    return temperature;
}