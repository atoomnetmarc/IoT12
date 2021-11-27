/*

Copyright 2021 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#include <Wire.h>

#include <EepromAbstractionWire.h>
#include <CRC.h>

#include "config.h"
#include "Setting.h"

I2cAt24Eeprom anEeprom(I2C_ADDRESS_EEPROM, PAGESIZE_AT24C32);

SettingClass::SettingClass()
{
}

void SettingClass::init(void)
{
    _load(false);
}

void SettingClass::loop(void)
{
    static unsigned long saveTimer = 0;

    if (_saveRequested)
    {
        _saveRequested = false;
        saveTimer = millis() + 500; // (re)schedule save over 0.5 seconds in case more settings are changed.
    }

    if (saveTimer > 0 && saveTimer < millis())
    {
        saveTimer = 0;
        _save();
    }
}

void SettingClass::defaults(void)
{
    _load(true);
}

void SettingClass::save(void)
{
    _saveRequested = true;
}

void SettingClass::_save(void)
{
    Setting_u setting_union;
    memset(setting_union.m_bytes, 0, sizeof(setting_union.m_bytes));

    setting_union.m_Setting.targetTemperatureHeater = targetTemperatureHeater;
    setting_union.m_Setting.gainVSupplyResistorDivider = gainVSupplyResistorDivider;
    setting_union.m_Setting.gainHeaterTemperatureAmplifier = gainHeaterTemperatureAmplifier;
    setting_union.m_Setting.gainHeaterCurrentAmplifier = gainHeaterCurrentAmplifier;
    setting_union.m_Setting.resistanceHeaterShunt = resistanceHeaterShunt;
    setting_union.m_Setting.resistanceNTCPullUp = resistanceNTCPullUp;
    setting_union.m_Setting.i2sGain = i2sGain;

    setting_union.m_Setting.settingSize = sizeof(setting_union.m_bytes);
    setting_union.m_Setting.settingCRC = crc32(setting_union.m_bytes, setting_union.m_Setting.settingSize);

    anEeprom.writeArrayToRom(ROM_START, setting_union.m_bytes, setting_union.m_Setting.settingSize);

    SerialUSB.println("Wrote " + String(setting_union.m_Setting.settingSize) + " bytes to EEPROM.");
}

//Gets the setting stored in EEPROM or the default when data is corrupted.
void SettingClass::_load(bool defaults)
{
    Setting_u setting_union;

    //Always load defaults (for new members not in EEPROM).
    setting_union.m_Setting = _getDefaults();

    //Read in the settings size from EEPROM. The size is stored in native-endian (little for the SAMD21), however the read16() reads an uint16_t in big-endian, so we must bswap it.
    uint16_t settingSize = __builtin_bswap16(anEeprom.read16(ROM_START));

    if (defaults == false &&                         //Use defaults when requested.
        settingSize <= sizeof(setting_union.m_bytes) //If settings in EEPROM claim to be larger than the settings struct, return defaults.
    )
    {
        //Read the claimed setting size into the union.
        anEeprom.readIntoMemArray(setting_union.m_bytes, ROM_START, settingSize);

        //Check the CRC of the read settings.
        uint32_t claimedCRC = setting_union.m_Setting.settingCRC;
        setting_union.m_Setting.settingCRC = 0; //CRC was calculated without the CRC value.
        uint32_t calculatedCRC = crc32(setting_union.m_bytes, settingSize);

        //Setting not valid or strange?
        if (calculatedCRC != claimedCRC)
        {
            //Then use defaults.
            setting_union.m_Setting = _getDefaults();
        }
        //Otherwise, we are good to go.
    }

    targetTemperatureHeater = setting_union.m_Setting.targetTemperatureHeater;
    gainVSupplyResistorDivider = setting_union.m_Setting.gainVSupplyResistorDivider;
    gainHeaterTemperatureAmplifier = setting_union.m_Setting.gainHeaterTemperatureAmplifier;
    gainHeaterCurrentAmplifier = setting_union.m_Setting.gainHeaterCurrentAmplifier;
    resistanceHeaterShunt = setting_union.m_Setting.resistanceHeaterShunt;
    resistanceNTCPullUp = setting_union.m_Setting.resistanceNTCPullUp;
    i2sGain = setting_union.m_Setting.i2sGain;
    thermocoupleInputOffsetVoltage = setting_union.m_Setting.thermocoupleInputOffsetVoltage;
}

Setting_t SettingClass::_getDefaults(void)
{
    Setting_t setting;

    setting.targetTemperatureHeater = 373.15;

    //Power supply voltage has been divided by a 100K in series with a 4K7 resistor.
    setting.gainVSupplyResistorDivider = 0.04489;

    setting.gainHeaterTemperatureAmplifier = 106.1;
    setting.gainHeaterCurrentAmplifier = 50.4;

    setting.resistanceHeaterShunt = 0.005;
    setting.resistanceNTCPullUp = 10000;

    setting.i2sGain = MAX98357_GAIN_9dB;

    //INA826 input offset voltage 40uV typical to 150uV maximum.
    setting.thermocoupleInputOffsetVoltage = 0.00004; //40uV.

    return setting;
}

SettingClass Setting;