/*

Copyright 2021-2022 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#include <Arduino.h>
#include <Wire.h>
#include <wiring_private.h>

#include "config.h"
#include "main.h"
#include "WireMinion.h"
#include "I2SGPIO.h"
#include "Setting.h"

void _requestEvent(void)
{
    WireMinion.requestEvent();
}

void _receiveEvent(int numBytes)
{
    WireMinion.receiveEvent(numBytes);
}

WireMinionClass::WireMinionClass()
{
}

void WireMinionClass::init(void)
{
    //Configure 2nd i2c port as a minion to our master controller.
    _minion.begin(I2C_MINION_ADDRESS);
    _minion.onRequest(_requestEvent);
    _minion.onReceive(_receiveEvent);
    pinPeripheral(2, PIO_SERCOM_ALT);
    pinPeripheral(3, PIO_SERCOM_ALT);

    pinMode(PIN_HEAT_INT, OUTPUT);

    raiseInterrupt(INT_REBOOT);
}

void WireMinionClass::loop(void)
{
    if (_assertHeatINT == true)
    {
        _assertHeatINT = false;
        digitalWrite(PIN_HEAT_INT, HIGH);
    }
}

void WireMinionClass::requestEvent(void)
{
    uint8_t sendBytes = 4;
    uint8_t bytes[16];
    float_u float_union;

    if (_i2cRegister == REGISTER_INTERRUPT)
    {
        uint32_t_u uint32_t_union;
        sendBytes = sizeof(uint32_t_union.m_bytes);
        uint32_t_union.m_uint32_t = _interruptFlags;
        memcpy(bytes, uint32_t_union.m_bytes, sendBytes);

        if (_interruptFlags == 0)
            digitalWrite(LED_BUILTIN, HIGH);
        else
            digitalWrite(LED_BUILTIN, LOW);

        _interruptFlags = 0;
        _assertHeatINT = false;
        digitalWrite(PIN_HEAT_INT, LOW);
    }
    else if (_i2cRegister == REGISTER_SERIAL_NUMBER)
    {
        SerialNumber_u SerialNumber_union;
        sendBytes = sizeof(SerialNumber_union.m_bytes);
        SerialNumber_union.m_SerialNumber.word0 = *(uint32_t *)0x0080A00C;
        SerialNumber_union.m_SerialNumber.word1 = *(uint32_t *)0x0080A040;
        SerialNumber_union.m_SerialNumber.word2 = *(uint32_t *)0x0080A044;
        SerialNumber_union.m_SerialNumber.word3 = *(uint32_t *)0x0080A048;
        memcpy(bytes, SerialNumber_union.m_bytes, sendBytes);
    }
    else if (_i2cRegister == REGISTER_TEMPERATURE_HEATER_TARGET)
    {
        sendBytes = sizeof(float_union.m_bytes);
        float_union.m_float = Setting.targetTemperatureHeater;
        memcpy(bytes, float_union.m_bytes, sendBytes);
    }
    else if (_i2cRegister == REGISTER_TEMPERATURE_HEATER)
    {
        sendBytes = sizeof(float_union.m_bytes);
        float_union.m_float = tTip;
        memcpy(bytes, float_union.m_bytes, sendBytes);
    }
    else if (_i2cRegister == REGISTER_TEMPERATURE_NTC)
    {
        sendBytes = sizeof(float_union.m_bytes);
        float_union.m_float = tAmbient;
        memcpy(bytes, float_union.m_bytes, sendBytes);
    }
    else if (_i2cRegister == REGISTER_I2S_SD)
    {
        sendBytes = 1;
        bytes[0] = I2SGPIO.getEnable();
    }
    else if (_i2cRegister == REGISTER_I2S_GAIN)
    {
        sendBytes = 1;
        bytes[0] = I2SGPIO.getGain();
    }
    else if (_i2cRegister == REGISTER_VERSION)
    {
        FirmwareVersion_u fv;
        sendBytes = sizeof(fv.m_bytes);

        fv.m_FirmwareVersion.x = VERSION_MAYOR;
        fv.m_FirmwareVersion.y = VERSION_MINOR;
        fv.m_FirmwareVersion.z = VERSION_PATCH;
        memcpy(bytes, fv.m_bytes, sendBytes);
    }
    else if (_i2cRegister == REGISTER_CONST1)
    {
        uint16_t_u uint16_t_union;
        uint16_t_union.m_uint16_t = 0x1234;
        sendBytes = sizeof(uint16_t_union.m_bytes);
        memcpy(bytes, uint16_t_union.m_bytes, sendBytes);
    }
    else if (_i2cRegister == REGISTER_CONST2)
    {
        sendBytes = sizeof(float_union.m_bytes);
        float_union.m_float = PI;
        memcpy(bytes, float_union.m_bytes, sendBytes);
    }
    else if (_i2cRegister == REGISTER_HEATER_STATE)
    {
        sendBytes = sizeof(HEATER_STATE);
        bytes[0] = Setting.heaterState;
    }
    else if (_i2cRegister == REGISTER_GAIN_VSUPPLY_RESISTOR_DIVIDER)
    {
        sendBytes = sizeof(float_union.m_bytes);
        float_union.m_float = Setting.gainVSupplyResistorDivider;
        memcpy(bytes, float_union.m_bytes, sendBytes);
    }
    else if (_i2cRegister == REGISTER_GAIN_HEATER_TEMPERATURE_AMPLIFIER)
    {
        sendBytes = sizeof(float_union.m_bytes);
        float_union.m_float = Setting.gainHeaterTemperatureAmplifier;
        memcpy(bytes, float_union.m_bytes, sendBytes);
    }
    else if (_i2cRegister == REGISTER_GAIN_HEATER_CURRENT_AMPLIFIER)
    {
        sendBytes = sizeof(float_union.m_bytes);
        float_union.m_float = Setting.gainHeaterCurrentAmplifier;
        memcpy(bytes, float_union.m_bytes, sendBytes);
    }
    else if (_i2cRegister == REGISTER_RESISTOR_NTC_PULLUP)
    {
        sendBytes = sizeof(float_union.m_bytes);
        float_union.m_float = Setting.resistanceNTCPullUp;
        memcpy(bytes, float_union.m_bytes, sendBytes);
    }
    else if (_i2cRegister == REGISTER_RESISTOR_HEATER_SHUNT)
    {
        sendBytes = sizeof(float_union.m_bytes);
        float_union.m_float = Setting.resistanceHeaterShunt;
        memcpy(bytes, float_union.m_bytes, sendBytes);
    }

    for (uint8_t i = 0; i < sendBytes; i++)
    {
        _minion.write(bytes[i]);
    }
}

void WireMinionClass::receiveEvent(int numBytes)
{
    float_u float_union;

    //Read register address
    if (_minion.available())
        _i2cRegister = _minion.read();

    //Read up to sizeof(_i2cData), ignore the rest.
    _i2cDataSize = 0;
    uint8_t downcounter = (sizeof(_i2cData) - 1);
    while (_minion.available())
    {
        uint8_t b = _minion.read();

        if (downcounter > 0)
        {
            _i2cData[downcounter] = b;
            downcounter--;
            _i2cDataSize++;
        }
    }

    //If not data received, do nothing.
    if (_i2cDataSize == 0)
        return;

    //Shift data to first element in array for easy processing.
    if (downcounter > 0)
    {
        for (uint8_t i = 0; i < _i2cDataSize; i++)
            _i2cData[i] = _i2cData[i + downcounter + 1];

        //Erase the remaining bytes for more easy debugging
        for (uint8_t i = _i2cDataSize; i < sizeof(_i2cData); i++)
            _i2cData[i] = 0;
    }

    if (_i2cRegister == REGISTER_HEATER_STATE && _i2cDataSize == sizeof(HEATER_STATE))
    {
        Setting.heaterState = (HEATER_STATE)_i2cData[0];
    }
    else if (_i2cRegister == REGISTER_TEMPERATURE_HEATER_TARGET && _i2cDataSize == sizeof(float_union.m_bytes))
    {
        memcpy(float_union.m_bytes, _i2cData, sizeof(float_union.m_bytes));

        //Max temperature is 725K;
        if (float_union.m_float > 725)
            float_union.m_float = 725;

        Setting.targetTemperatureHeater = float_union.m_float;
        Setting.save();
    }
    else if (_i2cRegister == REGISTER_GAIN_VSUPPLY_RESISTOR_DIVIDER && _i2cDataSize == sizeof(float_union.m_bytes))
    {
        memcpy(float_union.m_bytes, _i2cData, sizeof(float_union.m_bytes));
        Setting.gainVSupplyResistorDivider = float_union.m_float;
        Setting.save();
    }
    else if (_i2cRegister == REGISTER_GAIN_HEATER_TEMPERATURE_AMPLIFIER && _i2cDataSize == sizeof(float_union.m_bytes))
    {
        memcpy(float_union.m_bytes, _i2cData, sizeof(float_union.m_bytes));
        Setting.gainHeaterTemperatureAmplifier = float_union.m_float;
        Setting.save();
    }
    else if (_i2cRegister == REGISTER_GAIN_HEATER_CURRENT_AMPLIFIER && _i2cDataSize == sizeof(float_union.m_bytes))
    {
        memcpy(float_union.m_bytes, _i2cData, sizeof(float_union.m_bytes));
        Setting.gainHeaterCurrentAmplifier = float_union.m_float;
        Setting.save();
    }
    else if (_i2cRegister == REGISTER_RESISTOR_NTC_PULLUP && _i2cDataSize == sizeof(float_union.m_bytes))
    {
        memcpy(float_union.m_bytes, _i2cData, sizeof(float_union.m_bytes));
        Setting.resistanceNTCPullUp = float_union.m_float;
        Setting.save();
    }
    else if (_i2cRegister == REGISTER_RESISTOR_HEATER_SHUNT && _i2cDataSize == sizeof(float_union.m_bytes))
    {
        memcpy(float_union.m_bytes, _i2cData, sizeof(float_union.m_bytes));
        Setting.resistanceHeaterShunt = float_union.m_float;
        Setting.save();
    }
    else if (_i2cRegister == REGISTER_I2S_GAIN && _i2cDataSize == 1)
    {
        I2SGPIO.setGain((MAX98357_GAIN)_i2cData[0]);
    }
    else if (_i2cRegister == REGISTER_I2S_SD && _i2cDataSize == 1)
    {
        I2SGPIO.setEnable((bool)_i2cData[0]);
    }
}

void WireMinionClass::raiseInterrupt(uint8_t flag)
{
    if (flag > 0 && flag < 32)
    {
        _interruptFlags |= (1 << flag);
        _assertHeatINT = true;
    }
}

void WireMinionClass::onService(void)
{
    _minion.onService();
}

WireMinionClass WireMinion;

void SERCOM2_Handler(void)
{
    WireMinion.onService();
}
