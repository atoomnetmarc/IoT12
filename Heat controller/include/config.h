/*

Copyright 2021 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#ifndef config_h
#define config_h

#include <Arduino.h>

//Max 2047
#define VERSION_MAYOR 0u

//Max 32767
#define VERSION_MINOR 1u

//Max 63
#define VERSION_PATCH 2u

#define PIN_HEAT_INT (4u)
#define PIN_HEAT_PWM (11u)

#define PIN_I2S_GAIN0 (5u)
#define PIN_I2S_GAIN1 (6u)
#define PIN_I2S_SD (7u)

#define PIN_ADC_READY (8u)

#define PIN_INTERNAL_ADC_NTC PIN_A0
#define PIN_INTERNAL_ADC_TEMP PIN_A1
#define PIN_INTERNAL_ADC_AIN_COMP PIN_A2
#define PIN_INTERNAL_ADC_VIN PIN_A4
#define PIN_INTERNAL_ADC_IRON_CURRENT PIN_A5

#define AIN_EXTERNAL_ADC_TEMP (0u)
#define AIN_EXTERNAL_ADC_NTC (1u)
#define AIN_EXTERNAL_ADC_IRON_CURRENT (2u)
#define AIN_EXTERNAL_ADC_VIN (3u)

#define I2C_ADDRESS_ADS1115 0x48
#define I2C_ADDRESS_EEPROM 0x50

#endif