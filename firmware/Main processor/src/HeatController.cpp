/*

Copyright 2021 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#include "config.h"
#include "HeatController.h"

void IRAM_ATTR HeatControllerINT()
{
    HeatController._INT();
}

HeatControllerClass::HeatControllerClass()
{
    i2cAddress = 0x2A;
}

bool HeatControllerClass::init()
{
    pinMode(PIN_HEAT_INT, INPUT);
    attachInterrupt(PIN_HEAT_INT, HeatControllerINT, RISING);

    //Process interrupt that may be raised before boot.
    if (digitalRead(PIN_HEAT_INT) == HIGH)
        HeatControllerINT();

    Wire.beginTransmission(i2cAddress);
    uint8_t success = Wire.endTransmission();
    if (success)
    {
        return 0;
    }

    return 1;
}

void HeatControllerClass::loop()
{
    if (this->heatInt == true)
    {
        this->heatInt = false;
        uint32_t interrupt = getInterrupt();

        Serial.print("HEAT_INT " + String(((float)millis()) / 1000, 2) + "s : ");
        if (interrupt == 0)
        {
            Serial.print("0");
        }
        else
        {
            Serial.print("0b");
            for (int8_t b = 31; b >= 0; b--)
            {

                if (interrupt & (1 << b))
                    Serial.print("1");
                else
                    Serial.print("0");

                if (b % 8 == 0 && b > 0)
                    Serial.print(".");
            }
        }
        Serial.println();
    }
}

float HeatControllerClass::getTargetTemperature(void)
{
    return readRegister_float(REGISTER_TEMPERATURE_HEATER_TARGET);
}

void HeatControllerClass::setTargetTemperature(float kelvin)
{
    writeRegister(REGISTER_TEMPERATURE_HEATER_TARGET, kelvin);
}

HEATER_STATE HeatControllerClass::getHeaterState(void)
{
    return (HEATER_STATE)readRegister_uint8_t(REGISTER_HEATER_STATE);
}

void HeatControllerClass::setHeaterState(HEATER_STATE state)
{
    writeRegister(REGISTER_HEATER_STATE, (uint8_t)state);
}

float HeatControllerClass::getTemperatureTip(void)
{
    return readRegister_float(REGISTER_TEMPERATURE_HEATER);
}

float HeatControllerClass::getTemperatureNTC(void)
{
    return readRegister_float(REGISTER_TEMPERATURE_NTC);
}

void HeatControllerClass::setI2SEnable(bool enable)
{
    writeRegister(REGISTER_I2S_SD, (uint8_t)(enable));
}

void HeatControllerClass::setI2SGain(MAX98357_GAIN gain)
{
    writeRegister(REGISTER_I2S_GAIN, (uint8_t)(gain));
}

FirmwareVersion_t HeatControllerClass::getFirmwareVersion(void)
{
    FirmwareVersion_u fv;
    readRegisterArray(REGISTER_VERSION, fv.m_bytes, sizeof(fv.m_bytes));
    return fv.m_FirmwareVersion;
}

SerialNumber_t HeatControllerClass::getSerialNumber(void)
{
    SerialNumber_u sn;
    readRegisterArray(REGISTER_SERIAL_NUMBER, sn.m_bytes, sizeof(sn.m_bytes));
    return sn.m_SerialNumber;
}

uint32_t HeatControllerClass::getInterrupt(void)
{
    uint32_t_u interrupt;
    readRegisterArray(REGISTER_INTERRUPT, interrupt.m_bytes, sizeof(interrupt.m_bytes));
    return interrupt.m_uint32_t;
}

uint16_t HeatControllerClass::getConst1(void)
{
    uint16_t_u const1;
    readRegisterArray(REGISTER_CONST1, const1.m_bytes, sizeof(const1.m_bytes));
    return const1.m_uint16_t;
}

float HeatControllerClass::getConst2(void)
{
    float_u const2;
    readRegisterArray(REGISTER_CONST2, const2.m_bytes, sizeof(const2.m_bytes));
    return const2.m_float;
}

void HeatControllerClass::_INT()
{
    this->heatInt = true;
}

void HeatControllerClass::selectRegister(uint8_t r)
{
    Wire.beginTransmission(i2cAddress);
    Wire.write(r);
    Wire.endTransmission();
}

void HeatControllerClass::readRegisterArray(uint8_t r, uint8_t *dest, size_t len)
{
    selectRegister(r);
    Wire.requestFrom(i2cAddress, len);
    for (size_t b = 0; b < len; b++)
    {
        dest[b] = Wire.read();
    }
}

uint8_t HeatControllerClass::readRegister_uint8_t(uint8_t r)
{
    uint8_t_u uint8_t_union;
    readRegisterArray(r, uint8_t_union.m_bytes, sizeof(uint8_t_union.m_bytes));
    return uint8_t_union.m_uint8_t;
}

uint32_t HeatControllerClass::readRegister_uint32_t(uint8_t r)
{
    uint32_t_u uint32_t_union;
    readRegisterArray(r, uint32_t_union.m_bytes, sizeof(uint32_t_union.m_bytes));
    return uint32_t_union.m_uint32_t;
}

float HeatControllerClass::readRegister_float(uint8_t r)
{
    float_u float_union;
    readRegisterArray(r, float_union.m_bytes, sizeof(float_union.m_bytes));
    return float_union.m_float;
}

uint8_t HeatControllerClass::writeRegister(uint8_t r, uint8_t *src, size_t len)
{
    Wire.beginTransmission(i2cAddress);
    Wire.write(r);

    for (int8_t b = len; b > 0; b--)
        Wire.write(src[b - 1]);

    return Wire.endTransmission();
}

uint8_t HeatControllerClass::writeRegister(uint8_t r, uint8_t v)
{
    return writeRegister(r, &v, sizeof(v));
}

uint8_t HeatControllerClass::writeRegister(uint8_t r, uint16_t v)
{
    uint16_t_u uint16_t_union;
    uint16_t_union.m_uint16_t = v;
    return writeRegister(r, uint16_t_union.m_bytes, sizeof(uint16_t_union.m_uint16_t));
}

uint8_t HeatControllerClass::writeRegister(uint8_t r, uint32_t v)
{
    uint32_t_u uint32_t_union;
    uint32_t_union.m_uint32_t = v;
    return writeRegister(r, uint32_t_union.m_bytes, sizeof(uint32_t_union.m_uint32_t));
}

uint8_t HeatControllerClass::writeRegister(uint8_t r, uint64_t v)
{
    uint64_t_u uint64_t_union;
    uint64_t_union.m_uint64_t = v;
    return writeRegister(r, uint64_t_union.m_bytes, sizeof(uint64_t_union.m_uint64_t));
}

uint8_t HeatControllerClass::writeRegister(uint8_t r, float v)
{
    float_u float_union;
    float_union.m_float = v;
    return writeRegister(r, float_union.m_bytes, sizeof(float_union.m_float));
}

HeatControllerClass HeatController;
