/*

Copyright 2021 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#ifndef Setting_H
#define Setting_H

#include "I2SGPIO.h"

//Settings are stored in EEPROM from address 0x0400 to 0x0FFF. 0x0000 to 0x03FF are kept free.
#define ROM_START 0x0400

typedef struct __attribute__((packed))
{
    uint16_t settingSize;
    uint32_t settingCRC; //Provides some protection against data corruption, but does nothing in case the fields of this stuct are shuffled.

    /*

    Be carefull to only add new members to the end of the struct in order to be backward compatible with older settings structs stored in EEPROM.

    The variables of initial git commit are shuffled once to hide the non-sortability of future members.

    Resist sorting members.

    */

    float resistanceNTCPullUp;
    float gainHeaterCurrentAmplifier;
    MAX98357_GAIN i2sGain;
    float gainHeaterTemperatureAmplifier;
    float gainVSupplyResistorDivider;
    float targetTemperatureHeater;
    float resistanceHeaterShunt;

} Setting_t;

union Setting_u
{
    Setting_t m_Setting;
    uint8_t m_bytes[sizeof(Setting_t)];
};

enum HEATER_STATE : uint8_t
{
    OFF = 0x00,
    ON = 0x01,
};

class SettingClass
{
public:
    SettingClass();
    void init(void);
    void loop(void);
    void defaults(void);
    void save();

    HEATER_STATE heaterState = HEATER_STATE::OFF;
    float targetTemperatureHeater;

    float gainVSupplyResistorDivider;
    float gainHeaterTemperatureAmplifier;
    float gainHeaterCurrentAmplifier;

    float resistanceHeaterShunt;
    float resistanceNTCPullUp;

    MAX98357_GAIN i2sGain;

private:
    void _save();
    void _load(bool defaults);

    Setting_t _getDefaults(void);

    bool _saveRequested = false;
};

extern SettingClass Setting;

#endif