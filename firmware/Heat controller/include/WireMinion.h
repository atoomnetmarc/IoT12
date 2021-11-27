/*

Copyright 2021 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#ifndef WireMinion_H
#define WireMinion_H

#include <Wire.h>

#define I2C_MINION_ADDRESS 0x2A

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

class WireMinionClass
{
public:
    WireMinionClass();
    void init(void);
    void loop(void);
    void requestEvent(void);
    void receiveEvent(int numBytes);
    void onService(void);
    void raiseInterrupt(uint8_t flag);

private:
    uint8_t _i2cDataReceived = 0;
    uint8_t _i2cRegister = 0;
    uint8_t _i2cData[8];
    uint8_t _i2cDataSize = 0;

    uint32_t _interruptFlags = 0;

    bool _assertHeatINT = false;

    TwoWire _minion = TwoWire(&sercom2, 2, 3);
};

extern WireMinionClass WireMinion;

#endif