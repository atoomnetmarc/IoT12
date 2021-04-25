/*

Copyright 2021 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#ifndef HeatController_H_
#define HeatController_H_

#if (ARDUINO >= 100)
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Wire.h>

//System
#define REGISTER_VERSION 0x00
#define REGISTER_SERIAL_NUMBER 0x01
#define REGISTER_INTERRUPT 0x10
#define REGISTER_HEATER_STATE 0x11

//Measurements
#define REGISTER_TEMPERATURE_HEATER 0x40
#define REGISTER_TEMPERATURE_NTC 0x41

//Configuration
#define REGISTER_TEMPERATURE_HEATER_TARGET 0x80
#define REGISTER_RESISTOR_NTC_PULLUP 0x90
#define REGISTER_RESISTOR_HEATER_SHUNT 0x91
#define REGISTER_GAIN_VSUPPLY_RESISTOR_DIVIDER 0xA0
#define REGISTER_GAIN_HEATER_TEMPERATURE_AMPLIFIER 0xA1
#define REGISTER_GAIN_HEATER_CURRENT_AMPLIFIER 0xA2

//Miscellaneous
#define REGISTER_I2S_GAIN 0xF0
#define REGISTER_I2S_SD 0xF1
#define REGISTER_CONST1 0xF8
#define REGISTER_CONST2 0xF9

#define INT_REBOOT 31
#define INT_MEASUREMENTS_GROUP_A 30
#define INT_MEASUREMENTS_GROUP_B 29

enum MAX98357_GAIN : uint8_t
{
    MAX98357_GAIN_3dB = 0x00,
    MAX98357_GAIN_6dB = 0x01,
    MAX98357_GAIN_9dB = 0x02,
    MAX98357_GAIN_12dB = 0x03,
    MAX98357_GAIN_15dB = 0x04,
};

enum HEATER_STATE: uint8_t {
    OFF = 0x00,
    ON = 0x01,
};

typedef struct __attribute__((packed))
{
    uint16_t x : 11;
    uint16_t y : 15;
    uint8_t z : 6;
} FirmwareVersion_t;

union FirmwareVersion_u
{
    FirmwareVersion_t m_FirmwareVersion;
    uint8_t m_bytes[sizeof(FirmwareVersion_t)];
};

struct SerialNumber
{
    uint32_t word0;
    uint32_t word1;
    uint32_t word2;
    uint32_t word3;
};

typedef struct
{
    uint32_t word0;
    uint32_t word1;
    uint32_t word2;
    uint32_t word3;
} SerialNumber_t;

union SerialNumber_u
{
    SerialNumber_t m_SerialNumber;
    uint8_t m_bytes[sizeof(SerialNumber_t)];
};

union float_u
{
    float m_float;
    uint8_t m_bytes[sizeof(float)];
};

union uint8_t_u
{
    uint8_t m_uint8_t;
    uint8_t m_bytes[sizeof(uint8_t)];
};

union uint16_t_u
{
    uint16_t m_uint16_t;
    uint8_t m_bytes[sizeof(uint16_t)];
};

union uint32_t_u
{
    uint32_t m_uint32_t;
    uint8_t m_bytes[sizeof(uint32_t)];
};

union uint64_t_u
{
    uint64_t m_uint64_t;
    uint8_t m_bytes[sizeof(uint64_t)];
};

class HeatControllerClass
{
public:
    HeatControllerClass();

    bool init();
    void loop(void);

    float getTargetTemperature(void);
    void setTargetTemperature(float kelvin);
    HEATER_STATE getHeaterState(void);
    void setHeaterState(HEATER_STATE state);

    float getTemperatureTip(void);
    float getTemperatureNTC(void);

    FirmwareVersion_t getFirmwareVersion(void);
    SerialNumber_t getSerialNumber(void);
    uint32_t getInterrupt(void);

    uint16_t getConst1(void);
    float getConst2(void);

    void setI2SGain(MAX98357_GAIN gain);
    void setI2SEnable(bool enable);

    void _INT(void);

private:
    int i2cAddress;
    bool heatInt = 0;

    void selectRegister(uint8_t r);

    void readRegisterArray(uint8_t r, uint8_t *dest, size_t len);
    uint8_t readRegister_uint8_t(uint8_t r);
    uint32_t readRegister_uint32_t(uint8_t r);
    float readRegister_float(uint8_t r);

    uint8_t writeRegister(uint8_t r, uint8_t *src, size_t len);
    uint8_t writeRegister(uint8_t r, uint8_t v);
    uint8_t writeRegister(uint8_t r, uint16_t v);
    uint8_t writeRegister(uint8_t r, uint32_t v);
    uint8_t writeRegister(uint8_t r, uint64_t v);
    uint8_t writeRegister(uint8_t r, float v);
};

extern HeatControllerClass HeatController;

#endif
