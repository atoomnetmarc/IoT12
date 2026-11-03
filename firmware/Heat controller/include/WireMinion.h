/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>

#include <Wire.h>

#define I2C_MINION_ADDRESS 0x2A

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
#define LINK_TIMEOUT_DEFAULT_MS 5000

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

union uint16_t_u {
    uint16_t m_uint16_t;
    uint8_t m_bytes[sizeof(uint16_t)];
};

union uint32_t_u {
    uint32_t m_uint32_t;
    uint8_t m_bytes[sizeof(uint32_t)];
};

/*
 * One entry of the register map. The CLI prints the map and the current value
 * of every register from this table.
 */
struct RegisterInfo {
    const char *name;
    uint8_t address;
    const char *type;
    uint8_t size;
    bool writable;
};

class WireMinionClass {
public:
    WireMinionClass();
    void init(void);
    void loop(void);
    void requestEvent(void);
    void receiveEvent(int numBytes);
    void onService(void);
    void raiseInterrupt(uint8_t flag);
    void clearInterrupts(void);

    uint32_t interruptFlags(void) const;
    bool heatIntAsserted(void) const;

    uint8_t readRegister(uint8_t reg, uint8_t *dest, uint8_t maxLength);
    bool writeRegister(uint8_t reg, const uint8_t *data, uint8_t length);

    size_t registerCount(void) const;
    const RegisterInfo &registerAt(size_t index) const;

    void setLinkTimeout(uint32_t ms);
    uint32_t linkTimeout(void) const;
    uint32_t lastHeartbeatAgeMs(void) const;
    bool linkTimedOut(void) const;
    uint32_t forcedOffCount(void) const;

    uint8_t lastRegister(void) const;
    bool lastWasRead(void) const;
    uint8_t lastLength(void) const;
    uint32_t lastTransactionAgeMs(void) const;

private:
    uint8_t _i2cRegister = 0;
    uint8_t _i2cData[8];
    uint8_t _i2cDataSize = 0;

    uint32_t _interruptFlags = 0;

    bool _assertHeatINT = false;

    uint32_t _lastHeartbeatMs = 0;
    uint32_t _linkTimeoutMs = LINK_TIMEOUT_DEFAULT_MS;
    bool _linkTimedOut = false;
    uint32_t _forcedOffCount = 0;

    uint8_t _lastRegister = 0;
    bool _lastWasRead = false;
    uint8_t _lastLength = 0;
    uint32_t _lastTransactionMs = 0;

    // Minion bus on PA08/PA09 (SERCOM2), D2/D3.
    TwoWire _minion = TwoWire(&sercom2, 2, 3);
};

extern WireMinionClass WireMinion;
