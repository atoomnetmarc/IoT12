/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <Arduino.h>

#define PIN_HEAT_PWM  (10u)
#define PIN_24V_ON    (9u)
#define PIN_ADC_READY (8u)
#define PIN_HEAT_INT  (4u)
#define PIN_FLASH_CS  (6u)
#define PIN_SHAKE     A3

#define PIN_INTERNAL_ADC_NTC          PIN_A0
#define PIN_INTERNAL_ADC_TEMP         PIN_A1
#define PIN_INTERNAL_ADC_VIN          PIN_A4
#define PIN_INTERNAL_ADC_IRON_CURRENT PIN_A5

#define AIN_EXTERNAL_ADC_TEMP         (0u)
#define AIN_EXTERNAL_ADC_NTC          (1u)
#define AIN_EXTERNAL_ADC_IRON_CURRENT (2u)
#define AIN_EXTERNAL_ADC_VIN          (3u)

#define I2C_ADDRESS_ADS1115 0x48
