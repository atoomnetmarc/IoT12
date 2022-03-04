/*

Copyright 2021-2022 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#ifndef I2S_H
#define I2S_H

#include <Arduino.h>

enum MAX98357_GAIN : uint8_t
{
    MAX98357_GAIN_3dB = 0x00,
    MAX98357_GAIN_6dB = 0x01,
    MAX98357_GAIN_9dB = 0x02,
    MAX98357_GAIN_12dB = 0x03,
    MAX98357_GAIN_15dB = 0x04,
};

class I2SGPIOClass
{
public:
    I2SGPIOClass();
    void init(void);
    void setEnable(bool enable);
    bool getEnable(void);
    void setGain(MAX98357_GAIN gain);
    MAX98357_GAIN getGain(void);

private:
    bool enabled;
    MAX98357_GAIN gain;
};

extern I2SGPIOClass I2SGPIO;

#endif