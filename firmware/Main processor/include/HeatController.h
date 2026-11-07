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
#define REGISTER_VERSION          0x00
#define REGISTER_SERIAL_NUMBER    0x01
#define REGISTER_UPTIME           0x02
#define REGISTER_RESET_CAUSE      0x03
#define REGISTER_FREE_RAM         0x04
#define REGISTER_EVENT_COUNT      0x05
#define REGISTER_BOARD            0x06
#define REGISTER_BUILD            0x07
#define REGISTER_SKETCH_SIZE      0x08
#define REGISTER_FREE_SKETCH      0x09
#define REGISTER_INTERRUPT        0x10
#define REGISTER_HEATER_STATE     0x11
#define REGISTER_LINK_TIMEOUT     0x12
#define REGISTER_FORCED_OFF_COUNT 0x13
#define FIRMWARE_VERSION_LENGTH   16

// Measurements
#define REGISTER_TEMPERATURE_HEATER   0x40
#define REGISTER_TEMPERATURE_NTC      0x41
#define REGISTER_HEATER_DUTY          0x42
#define REGISTER_SUPPLY_VOLTAGE       0x43
#define REGISTER_IRON_CURRENT         0x44
#define REGISTER_VOLTAGE_NTC          0x45
#define REGISTER_VOLTAGE_THERMOCOUPLE 0x46
#define REGISTER_VOLTAGE_VIN_ON       0x48
#define REGISTER_CURRENT_IRON_OFF     0x49
#define REGISTER_CURRENT_IRON_ON      0x4A
#define REGISTER_RESISTANCE_TIP       0x4B
#define REGISTER_RESISTANCE_SUPPLY    0x4C

// Heater control
#define REGISTER_CYCLE_COUNT 0x50
#define REGISTER_ADC_READ_MS 0x51
#define REGISTER_PID_TERMS   0x52
#define REGISTER_PID_GAINS   0x53
#define REGISTER_PWM_RAW     0x54

// Shake
#define REGISTER_SHAKE_VOLTAGE         0x60
#define REGISTER_SHAKE_STATE           0x61
#define REGISTER_SHAKE_LAST_MOTION_AGE 0x62

// Internal ADC
#define REGISTER_DIE_TEMPERATURE 0x70
#define REGISTER_VDD             0x71

// Configuration
#define REGISTER_TEMPERATURE_HEATER_TARGET         0x80
#define REGISTER_RESISTOR_NTC_PULLUP               0x90
#define REGISTER_RESISTOR_HEATER_SHUNT             0x91
#define REGISTER_THERMOCOUPLE_OFFSET               0x92
#define REGISTER_STANDBY_TEMP_1                    0x93
#define REGISTER_STANDBY_TEMP_2                    0x94
#define REGISTER_SHAKE_STAGE1_MS                   0x95
#define REGISTER_SHAKE_STAGE2_MS                   0x96
#define REGISTER_SHAKE_OFF_MS                      0x97
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

#define REGISTER_MAX_LENGTH   16
#define HEARTBEAT_INTERVAL_MS 1000

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
    float getVoltageNTC(void);
    float getVoltageThermocouple(void);
    float getVoltageVinOn(void);
    float getCurrentIronOff(void);
    float getCurrentIronOn(void);
    float getResistanceTip(void);
    float getResistanceSupply(void);
    float getGainHeaterTemperatureAmplifier(void);

    uint32_t getUptime(void);
    const char *getResetCauseName(void);
    uint32_t getFreeRam(void);
    uint32_t getSketchSize(void);
    uint32_t getFreeSketch(void);
    uint16_t getEventCount(void);
    const char *getBoard(void);
    const char *getBuild(void);
    uint32_t getForcedOffCount(void);

    uint32_t getCycleCount(void);
    uint32_t getAdcReadMs(void);
    void getPidTerms(float *p, float *i, float *d, float *heatfactor);
    void getPidGains(float *kp, float *ki, float *kd);
    void getPwmRaw(uint32_t *duty, uint32_t *period);

    float getShakeVoltage(void);
    void getShakeState(uint8_t *phase, bool *raw, bool *debounced, bool *expired);
    uint32_t getShakeLastMotionAge(void);

    float getDieTemperature(void);
    float getVdd(void);

    float getResistanceNtcPullup(void);
    float getResistanceHeaterShunt(void);
    float getGainVSupplyResistorDivider(void);
    float getGainHeaterCurrentAmplifier(void);
    float getThermocoupleOffset(void);
    float getStandbyTemp1(void);
    float getStandbyTemp2(void);
    float getShakeStage1Ms(void);
    float getShakeStage2Ms(void);
    float getShakeOffMs(void);

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
    bool linkUp(void) const;
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
    char boardName[FIRMWARE_VERSION_LENGTH] = {0};
    char buildDate[FIRMWARE_VERSION_LENGTH] = {0};

    HEATER_STATE cachedState = HEATER_STATE::OFF;
    HEATER_STATE desiredState = HEATER_STATE::OFF;
    bool desiredStateValid = false;
    uint32_t lastHeartbeatMs = 0;
    uint32_t lastInitAttemptMs = 0;
    bool linkTimedOutFlag = false;
    bool linkUpValue = false;
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
