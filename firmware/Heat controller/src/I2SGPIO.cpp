/*

Copyright 2021-2022 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#include "I2SGPIO.h"

#include "config.h"
#include "Setting.h"

I2SGPIOClass::I2SGPIOClass()
{
}

void I2SGPIOClass::init(void)
{
    this->setEnable(false);
    this->setGain(Setting.i2sGain);
}

void I2SGPIOClass::setEnable(bool enable)
{
    if (enable)
    {
        //Enable I2S amp.
        pinMode(PIN_I2S_SD, INPUT);
        digitalWrite(PIN_I2S_SD, 0);
    }
    else
    {
        //Shut down I2S amp.
        pinMode(PIN_I2S_SD, OUTPUT);
        digitalWrite(PIN_I2S_SD, 0);
    }

    this->enabled = enable;
}

bool I2SGPIOClass::getEnable(void)
{
    return this->enabled;
}

void I2SGPIOClass::setGain(MAX98357_GAIN gain)
{
    switch (gain)
    {
    case MAX98357_GAIN_15dB:
        pinMode(PIN_I2S_GAIN0, INPUT);
        pinMode(PIN_I2S_GAIN1, OUTPUT);
        digitalWrite(PIN_I2S_GAIN1, 0);
        break;
    case MAX98357_GAIN_12dB:
        pinMode(PIN_I2S_GAIN0, OUTPUT);
        pinMode(PIN_I2S_GAIN1, INPUT);
        digitalWrite(PIN_I2S_GAIN0, 0);
        break;
    case MAX98357_GAIN_9dB:
        pinMode(PIN_I2S_GAIN0, INPUT);
        pinMode(PIN_I2S_GAIN1, INPUT);
        break;
    case MAX98357_GAIN_6dB:
    case MAX98357_GAIN_3dB:
    default:
        pinMode(PIN_I2S_GAIN0, OUTPUT);
        pinMode(PIN_I2S_GAIN1, INPUT);
        digitalWrite(PIN_I2S_GAIN0, 1);
        break;

        /* This would be the 3dB selection but because the I2S amp is powered by 5V you cannot reach that voltage with the 3.3V output from the microcontroller, which is fine, just use one of the other gain settings.
        pinMode(PIN_I2S_GAIN0, INPUT);
        pinMode(PIN_I2S_GAIN1, OUTPUT);
        digitalWrite(PIN_I2S_GAIN1, 1);
        break;
        */
    }
    this->gain = gain;
}

MAX98357_GAIN I2SGPIOClass::getGain(void)
{
    return this->gain;
}

I2SGPIOClass I2SGPIO;
