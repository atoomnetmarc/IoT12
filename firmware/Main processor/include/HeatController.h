/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#if (ARDUINO >= 100)
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Wire.h>

// System
#define REGISTER_VERSION        0x00
#define REGISTER_SERIAL_NUMBER  0x01
#define REGISTER_INTERRUPT      0x10
#define REGISTER_HEATER_STATE   0x11
#define REGISTER_LINK_TIMEOUT   0x12
#define FIRMWARE_VERSION_LENGTH 16

// Measurements
#define REGISTER_TEMPERATURE_HEATER 0x40
#define REGISTER_TEMPERATURE_NTC    0x41
#define REGISTER_HEATER_DUTY        0x42
#define REGISTER_SUPPLY_VOLTAGE     0x43
#define REGISTER_IRON_CURRENT       0x44

// Configuration
#define REGISTER_TEMPERATURE_HEATER_TARGET         0x80
#define REGISTER_RESISTOR_NTC_PULLUP               0x90
#define REGISTER_RESISTOR_HEATER_SHUNT             0x91
#define REGISTER_GAIN_VSUPPLY_RESISTOR_DIVIDER     0xA0
#define REGISTER_GAIN_HEATER_TEMPERATURE_AMPLIFIER 0xA1
#define REGISTER_GAIN_HEATER_CURRENT_AMPLIFIER     0xA2

// Miscellaneous
#define REGISTER_CONST1 0xF8
#define REGISTER_CONST2 0xF9

#define INT_REBOOT               31
#define INT_MEASUREMENTS_GROUP_A 30
#define INT_MEASUREMENTS_GROUP_B 29
#define INT_LINK_TIMEOUT         28

#define REGISTER_MAX_LENGTH     16
#define HEARTBEAT_INTERVAL_MS   1000
#define LINK_TIMEOUT_DEFAULT_MS 5000

enum HEATER_STATE : uint8_t {
    OFF = 0x00,
    ON = 0x01,
};

typedef struct
{
    uint32_t word0;
    uint32_t word1;
    uint32_t word2;
    uint32_t word3;
} SerialNumber_t;

union SerialNumber_u {
    SerialNumber_t m_SerialNumber;
    uint8_t m_bytes[sizeof(SerialNumber_t)];
};

union float_u {
    float m_float;
    uint8_t m_bytes[sizeof(float)];
};

union uint8_t_u {
    uint8_t m_uint8_t;
    uint8_t m_bytes[sizeof(uint8_t)];
};

union uint16_t_u {
    uint16_t m_uint16_t;
    uint8_t m_bytes[sizeof(uint16_t)];
};

union uint32_t_u {
    uint32_t m_uint32_t;
    uint8_t m_bytes[sizeof(uint32_t)];
};

union uint64_t_u {
    uint64_t m_uint64_t;
    uint8_t m_bytes[sizeof(uint64_t)];
};

/*
 * One entry of the minion register map. The CLI prints the map and the current
 * value of every register from this table.
 */
struct RegisterInfo {
    const char *name;
    uint8_t address;
    const char *type;
    uint8_t size;
    bool writable;
};

class HeatControllerClass {
public:
    HeatControllerClass();

    bool init();
    void loop(void);

    float getTargetTemperature(void);
    uint8_t setTargetTemperature(float kelvin);
    HEATER_STATE getHeaterState(void);
    uint8_t setHeaterState(HEATER_STATE state);

    float getTemperatureTip(void);
    float getTemperatureNTC(void);
    float getHeaterDuty(void);
    float getSupplyVoltage(void);
    float getIronCurrent(void);

    const char *getFirmwareVersion(void);
    SerialNumber_t getSerialNumber(void);
    uint32_t getInterrupt(void);
    uint32_t getLastInterrupt(void);
    bool isCompatible(void);

    uint16_t getConst1(void);
    float getConst2(void);

    uint8_t readRegisterRaw(uint8_t r, uint8_t *dest, size_t len);
    uint8_t writeRegisterRaw(uint8_t r, const uint8_t *src, size_t len);

    size_t registerCount(void) const;
    const RegisterInfo &registerAt(size_t index) const;
    uint8_t registerSize(uint8_t r) const;

    uint32_t linkAgeMs(void) const;
    bool linkTimedOut(void) const;
    uint32_t rewriteCount(void) const;
    uint32_t errorCount(void) const;
    uint32_t heartbeatCount(void) const;
    uint32_t getLinkTimeout(void);
    uint8_t setLinkTimeout(uint32_t ms);

    void _INT(void);

private:
    int i2cAddress;
    bool heatInt = 0;
    bool compatible = false;
    uint32_t lastInterrupt = 0;
    char firmwareVersion[FIRMWARE_VERSION_LENGTH] = {0};

    HEATER_STATE cachedState = HEATER_STATE::OFF;
    HEATER_STATE desiredState = HEATER_STATE::OFF;
    bool desiredStateValid = false;
    uint32_t lastHeartbeatMs = 0;
    uint32_t lastInitAttemptMs = 0;
    bool linkTimedOutFlag = false;
    uint32_t rewriteCountValue = 0;
    uint32_t errorCountValue = 0;
    uint32_t heartbeatCountValue = 0;

    void heartbeat(void);
    uint8_t selectRegister(uint8_t r);
    bool checkCompatibility(void);

    size_t readRegisterArray(uint8_t r, uint8_t *dest, size_t len);
    uint8_t readRegister_uint8_t(uint8_t r);
    uint32_t readRegister_uint32_t(uint8_t r);
    float readRegister_float(uint8_t r);

    uint8_t writeRegister(uint8_t r, const uint8_t *src, size_t len);
    uint8_t writeRegister(uint8_t r, uint8_t v);
    uint8_t writeRegister(uint8_t r, uint16_t v);
    uint8_t writeRegister(uint8_t r, uint32_t v);
    uint8_t writeRegister(uint8_t r, uint64_t v);
    uint8_t writeRegister(uint8_t r, float v);
};

extern HeatControllerClass HeatController;
