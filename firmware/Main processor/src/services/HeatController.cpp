/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "HeatController.h"
#include "EventLog.h"
#include "Log.h"
#include "config.h"

static const char *tag = "HeatLink";

namespace {
    const RegisterInfo registers[] = {
        {"VERSION", REGISTER_VERSION, "str16", FIRMWARE_VERSION_LENGTH, false},
        {"SERIAL_NUMBER", REGISTER_SERIAL_NUMBER, "u32x4", 16, false},
        {"UPTIME", REGISTER_UPTIME, "u32", 4, false},
        {"RESET_CAUSE", REGISTER_RESET_CAUSE, "u8", 1, false},
        {"FREE_RAM", REGISTER_FREE_RAM, "u32", 4, false},
        {"EVENT_COUNT", REGISTER_EVENT_COUNT, "u16", 2, false},
        {"BOARD", REGISTER_BOARD, "str16", FIRMWARE_VERSION_LENGTH, false},
        {"BUILD", REGISTER_BUILD, "str16", FIRMWARE_VERSION_LENGTH, false},
        {"SKETCH_SIZE", REGISTER_SKETCH_SIZE, "u32", 4, false},
        {"FREE_SKETCH", REGISTER_FREE_SKETCH, "u32", 4, false},
        {"INTERRUPT", REGISTER_INTERRUPT, "u32", 4, false},
        {"HEATER_STATE", REGISTER_HEATER_STATE, "u8", 1, true},
        {"LINK_TIMEOUT", REGISTER_LINK_TIMEOUT, "u32", 4, true},
        {"FORCED_OFF_COUNT", REGISTER_FORCED_OFF_COUNT, "u32", 4, false},
        {"TEMPERATURE_HEATER", REGISTER_TEMPERATURE_HEATER, "float", 4, false},
        {"TEMPERATURE_NTC", REGISTER_TEMPERATURE_NTC, "float", 4, false},
        {"HEATER_DUTY", REGISTER_HEATER_DUTY, "float", 4, false},
        {"SUPPLY_VOLTAGE", REGISTER_SUPPLY_VOLTAGE, "float", 4, false},
        {"IRON_CURRENT", REGISTER_IRON_CURRENT, "float", 4, false},
        {"VOLTAGE_NTC", REGISTER_VOLTAGE_NTC, "float", 4, false},
        {"VOLTAGE_THERMOCOUPLE", REGISTER_VOLTAGE_THERMOCOUPLE, "float", 4, false},
        {"VOLTAGE_VIN_ON", REGISTER_VOLTAGE_VIN_ON, "float", 4, false},
        {"CURRENT_IRON_OFF", REGISTER_CURRENT_IRON_OFF, "float", 4, false},
        {"CURRENT_IRON_ON", REGISTER_CURRENT_IRON_ON, "float", 4, false},
        {"RESISTANCE_TIP", REGISTER_RESISTANCE_TIP, "float", 4, false},
        {"RESISTANCE_SUPPLY", REGISTER_RESISTANCE_SUPPLY, "float", 4, false},
        {"CYCLE_COUNT", REGISTER_CYCLE_COUNT, "u32", 4, false},
        {"ADC_READ_MS", REGISTER_ADC_READ_MS, "u32", 4, false},
        {"PID_TERMS", REGISTER_PID_TERMS, "floatx4", 16, false},
        {"PID_GAINS", REGISTER_PID_GAINS, "floatx4", 16, false},
        {"PWM_RAW", REGISTER_PWM_RAW, "u32x2", 8, false},
        {"SHAKE_VOLTAGE", REGISTER_SHAKE_VOLTAGE, "float", 4, false},
        {"SHAKE_STATE", REGISTER_SHAKE_STATE, "u8x4", 4, false},
        {"SHAKE_LAST_MOTION_AGE", REGISTER_SHAKE_LAST_MOTION_AGE, "u32", 4, false},
        {"DIE_TEMPERATURE", REGISTER_DIE_TEMPERATURE, "float", 4, false},
        {"VDD", REGISTER_VDD, "float", 4, false},
        {"TEMPERATURE_HEATER_TARGET", REGISTER_TEMPERATURE_HEATER_TARGET, "float", 4, true},
        {"RESISTOR_NTC_PULLUP", REGISTER_RESISTOR_NTC_PULLUP, "float", 4, true},
        {"RESISTOR_HEATER_SHUNT", REGISTER_RESISTOR_HEATER_SHUNT, "float", 4, true},
        {"THERMOCOUPLE_OFFSET", REGISTER_THERMOCOUPLE_OFFSET, "float", 4, false},
        {"STANDBY_TEMP_1", REGISTER_STANDBY_TEMP_1, "float", 4, false},
        {"STANDBY_TEMP_2", REGISTER_STANDBY_TEMP_2, "float", 4, false},
        {"SHAKE_STAGE1_MS", REGISTER_SHAKE_STAGE1_MS, "float", 4, false},
        {"SHAKE_STAGE2_MS", REGISTER_SHAKE_STAGE2_MS, "float", 4, false},
        {"SHAKE_OFF_MS", REGISTER_SHAKE_OFF_MS, "float", 4, false},
        {"GAIN_VSUPPLY_RESISTOR_DIVIDER", REGISTER_GAIN_VSUPPLY_RESISTOR_DIVIDER, "float", 4, true},
        {"GAIN_HEATER_TEMPERATURE_AMPLIFIER", REGISTER_GAIN_HEATER_TEMPERATURE_AMPLIFIER, "float", 4, true},
        {"GAIN_HEATER_CURRENT_AMPLIFIER", REGISTER_GAIN_HEATER_CURRENT_AMPLIFIER, "float", 4, true},
        {"CONST1", REGISTER_CONST1, "u16", 2, false},
        {"CONST2", REGISTER_CONST2, "float", 4, false},
    };
}

void IRAM_ATTR HeatControllerINT() {
    HeatController._INT();
}

HeatControllerClass::HeatControllerClass() {
    i2cAddress = 0x2A;
}

bool HeatControllerClass::init() {
    pinMode(PIN_HEAT_INT, INPUT);
    attachInterrupt(PIN_HEAT_INT, HeatControllerINT, RISING);

    // Process interrupt that may be raised before boot.
    if (digitalRead(PIN_HEAT_INT) == HIGH) {
        HeatControllerINT();
    }

    Wire.beginTransmission(i2cAddress);
    if (Wire.endTransmission()) {
        return false;
    }

    if (checkCompatibility() == false) {
        return false;
    }

    lastHeartbeatMs = millis();
    lastInitAttemptMs = millis();
    heartbeat();

    return true;
}

void HeatControllerClass::loop() {
    uint32_t now = millis();

    /*
     * The heat controller can still be in its bootloader when this processor
     * boots. Retry the handshake until it answers, so a reset of either side
     * never leaves the link down.
     */
    if (compatible == false) {
        if (now - lastInitAttemptMs >= 1000) {
            lastInitAttemptMs = now;
            init();
        }

        return;
    }

    if (this->heatInt == true) {
        this->heatInt = false;
        lastInterrupt = getInterrupt();

        if (lastInterrupt & (1UL << INT_REBOOT)) {
            LOG_W(tag, "heat controller rebooted, re-checking compatibility");
            checkCompatibility();
        }

        if (lastInterrupt & (1UL << INT_LINK_TIMEOUT)) {
            linkTimedOutFlag = true;
            eventlog::Log.record(eventlog::LINK_TIMEOUT, 0);
            LOG_W(tag, "heat controller link timeout");
        }
    }

    if (now - lastHeartbeatMs >= HEARTBEAT_INTERVAL_MS) {
        lastHeartbeatMs = now;
        heartbeat();
    }
}

/*
 * Reads REGISTER_HEATER_STATE. That read is the link heartbeat: it restarts the
 * heat controller watchdog. After a timeout the requested state is written
 * again, so heating resumes when the link returns.
 */
void HeatControllerClass::heartbeat(void) {
    uint8_t state = 0;
    size_t got = readRegisterArray(REGISTER_HEATER_STATE, &state, sizeof(state));

    if (got != sizeof(state)) {
        errorCountValue++;
        return;
    }

    heartbeatCountValue++;
    linkUpValue = true;
    cachedState = (HEATER_STATE)state;

    if (linkTimedOutFlag) {
        linkTimedOutFlag = false;

        if (desiredStateValid) {
            if (writeRegister(REGISTER_HEATER_STATE, static_cast<uint8_t>(desiredState)) == 0) {
                rewriteCountValue++;
            } else {
                errorCountValue++;
            }
        }
    }
}

float HeatControllerClass::getTargetTemperature(void) {
    return readRegister_float(REGISTER_TEMPERATURE_HEATER_TARGET);
}

uint8_t HeatControllerClass::setTargetTemperature(float kelvin) {
    return writeRegister(REGISTER_TEMPERATURE_HEATER_TARGET, kelvin);
}

HEATER_STATE HeatControllerClass::getHeaterState(void) {
    return cachedState;
}

uint8_t HeatControllerClass::setHeaterState(HEATER_STATE state) {
    uint8_t result = writeRegister(REGISTER_HEATER_STATE, static_cast<uint8_t>(state));

    if (result == 0) {
        desiredState = state;
        desiredStateValid = true;
        cachedState = state;
    }

    return result;
}

float HeatControllerClass::getTemperatureTip(void) {
    return readRegister_float(REGISTER_TEMPERATURE_HEATER);
}

float HeatControllerClass::getTemperatureNTC(void) {
    return readRegister_float(REGISTER_TEMPERATURE_NTC);
}

float HeatControllerClass::getHeaterDuty(void) {
    return readRegister_float(REGISTER_HEATER_DUTY);
}

float HeatControllerClass::getSupplyVoltage(void) {
    return readRegister_float(REGISTER_SUPPLY_VOLTAGE);
}

float HeatControllerClass::getIronCurrent(void) {
    return readRegister_float(REGISTER_IRON_CURRENT);
}

float HeatControllerClass::getVoltageNTC(void) {
    return readRegister_float(REGISTER_VOLTAGE_NTC);
}

float HeatControllerClass::getVoltageThermocouple(void) {
    return readRegister_float(REGISTER_VOLTAGE_THERMOCOUPLE);
}

float HeatControllerClass::getVoltageVinOn(void) {
    return readRegister_float(REGISTER_VOLTAGE_VIN_ON);
}

float HeatControllerClass::getCurrentIronOff(void) {
    return readRegister_float(REGISTER_CURRENT_IRON_OFF);
}

float HeatControllerClass::getCurrentIronOn(void) {
    return readRegister_float(REGISTER_CURRENT_IRON_ON);
}

float HeatControllerClass::getResistanceTip(void) {
    return readRegister_float(REGISTER_RESISTANCE_TIP);
}

float HeatControllerClass::getResistanceSupply(void) {
    return readRegister_float(REGISTER_RESISTANCE_SUPPLY);
}

float HeatControllerClass::getGainHeaterTemperatureAmplifier(void) {
    return readRegister_float(REGISTER_GAIN_HEATER_TEMPERATURE_AMPLIFIER);
}

uint32_t HeatControllerClass::getUptime(void) {
    return readRegister_uint32_t(REGISTER_UPTIME);
}

const char *HeatControllerClass::getResetCauseName(void) {
    uint8_t cause = readRegister_uint8_t(REGISTER_RESET_CAUSE);

    if (cause & 0x01) {
        return "POR";
    }
    if (cause & 0x02) {
        return "BOD12";
    }
    if (cause & 0x04) {
        return "BOD33";
    }
    if (cause & 0x10) {
        return "EXT";
    }
    if (cause & 0x20) {
        return "WDT";
    }
    if (cause & 0x40) {
        return "SYST";
    }

    return "UNKNOWN";
}

uint32_t HeatControllerClass::getFreeRam(void) {
    return readRegister_uint32_t(REGISTER_FREE_RAM);
}

uint32_t HeatControllerClass::getSketchSize(void) {
    return readRegister_uint32_t(REGISTER_SKETCH_SIZE);
}

uint32_t HeatControllerClass::getFreeSketch(void) {
    return readRegister_uint32_t(REGISTER_FREE_SKETCH);
}

uint16_t HeatControllerClass::getEventCount(void) {
    uint16_t_u value;

    if (readRegisterArray(REGISTER_EVENT_COUNT, value.m_bytes, sizeof(value.m_bytes)) != sizeof(value.m_bytes)) {
        return 0;
    }

    return value.m_uint16_t;
}

const char *HeatControllerClass::getBoard(void) {
    readRegisterArray(REGISTER_BOARD, (uint8_t *)boardName, sizeof(boardName));
    boardName[sizeof(boardName) - 1] = '\0';
    return boardName;
}

const char *HeatControllerClass::getBuild(void) {
    readRegisterArray(REGISTER_BUILD, (uint8_t *)buildDate, sizeof(buildDate));
    buildDate[sizeof(buildDate) - 1] = '\0';
    return buildDate;
}

uint32_t HeatControllerClass::getForcedOffCount(void) {
    return readRegister_uint32_t(REGISTER_FORCED_OFF_COUNT);
}

uint32_t HeatControllerClass::getCycleCount(void) {
    return readRegister_uint32_t(REGISTER_CYCLE_COUNT);
}

uint32_t HeatControllerClass::getAdcReadMs(void) {
    return readRegister_uint32_t(REGISTER_ADC_READ_MS);
}

void HeatControllerClass::getPidTerms(float *p, float *i, float *d, float *heatfactor) {
    float values[4] = {NAN, NAN, NAN, NAN};
    readRegisterArray(REGISTER_PID_TERMS, reinterpret_cast<uint8_t *>(values), sizeof(values));
    *p = values[0];
    *i = values[1];
    *d = values[2];
    *heatfactor = values[3];
}

void HeatControllerClass::getPidGains(float *kp, float *ki, float *kd) {
    float values[4] = {NAN, NAN, NAN, NAN};
    readRegisterArray(REGISTER_PID_GAINS, reinterpret_cast<uint8_t *>(values), sizeof(values));
    *kp = values[0];
    *ki = values[1];
    *kd = values[2];
}

void HeatControllerClass::getPwmRaw(uint32_t *duty, uint32_t *period) {
    uint32_t values[2] = {0, 0};
    readRegisterArray(REGISTER_PWM_RAW, reinterpret_cast<uint8_t *>(values), sizeof(values));
    *duty = values[0];
    *period = values[1];
}

float HeatControllerClass::getShakeVoltage(void) {
    return readRegister_float(REGISTER_SHAKE_VOLTAGE);
}

void HeatControllerClass::getShakeState(uint8_t *phase, bool *raw, bool *debounced, bool *expired) {
    uint8_t values[4] = {0, 0, 0, 0};
    readRegisterArray(REGISTER_SHAKE_STATE, values, sizeof(values));
    *phase = values[0];
    *raw = values[1] != 0;
    *debounced = values[2] != 0;
    *expired = values[3] != 0;
}

uint32_t HeatControllerClass::getShakeLastMotionAge(void) {
    return readRegister_uint32_t(REGISTER_SHAKE_LAST_MOTION_AGE);
}

float HeatControllerClass::getDieTemperature(void) {
    return readRegister_float(REGISTER_DIE_TEMPERATURE);
}

float HeatControllerClass::getVdd(void) {
    return readRegister_float(REGISTER_VDD);
}

float HeatControllerClass::getResistanceNtcPullup(void) {
    return readRegister_float(REGISTER_RESISTOR_NTC_PULLUP);
}

float HeatControllerClass::getResistanceHeaterShunt(void) {
    return readRegister_float(REGISTER_RESISTOR_HEATER_SHUNT);
}

float HeatControllerClass::getGainVSupplyResistorDivider(void) {
    return readRegister_float(REGISTER_GAIN_VSUPPLY_RESISTOR_DIVIDER);
}

float HeatControllerClass::getGainHeaterCurrentAmplifier(void) {
    return readRegister_float(REGISTER_GAIN_HEATER_CURRENT_AMPLIFIER);
}

float HeatControllerClass::getThermocoupleOffset(void) {
    return readRegister_float(REGISTER_THERMOCOUPLE_OFFSET);
}

float HeatControllerClass::getStandbyTemp1(void) {
    return readRegister_float(REGISTER_STANDBY_TEMP_1);
}

float HeatControllerClass::getStandbyTemp2(void) {
    return readRegister_float(REGISTER_STANDBY_TEMP_2);
}

float HeatControllerClass::getShakeStage1Ms(void) {
    return readRegister_float(REGISTER_SHAKE_STAGE1_MS);
}

float HeatControllerClass::getShakeStage2Ms(void) {
    return readRegister_float(REGISTER_SHAKE_STAGE2_MS);
}

float HeatControllerClass::getShakeOffMs(void) {
    return readRegister_float(REGISTER_SHAKE_OFF_MS);
}

const char *HeatControllerClass::getFirmwareVersion(void) {
    readRegisterArray(REGISTER_VERSION, (uint8_t *)firmwareVersion, sizeof(firmwareVersion));
    firmwareVersion[sizeof(firmwareVersion) - 1] = '\0';
    return firmwareVersion;
}

bool HeatControllerClass::checkCompatibility(void) {
    getFirmwareVersion();
    compatible = (strcmp(firmwareVersion, PRODUCT_VERSION) == 0);
    return compatible;
}

bool HeatControllerClass::isCompatible(void) {
    return compatible;
}

SerialNumber_t HeatControllerClass::getSerialNumber(void) {
    SerialNumber_u sn;
    readRegisterArray(REGISTER_SERIAL_NUMBER, sn.m_bytes, sizeof(sn.m_bytes));
    return sn.m_SerialNumber;
}

uint32_t HeatControllerClass::getInterrupt(void) {
    uint32_t_u interrupt;
    readRegisterArray(REGISTER_INTERRUPT, interrupt.m_bytes, sizeof(interrupt.m_bytes));
    return interrupt.m_uint32_t;
}

uint32_t HeatControllerClass::getLastInterrupt(void) {
    return lastInterrupt;
}

uint16_t HeatControllerClass::getConst1(void) {
    uint16_t_u const1;
    readRegisterArray(REGISTER_CONST1, const1.m_bytes, sizeof(const1.m_bytes));
    return const1.m_uint16_t;
}

float HeatControllerClass::getConst2(void) {
    float_u const2;
    readRegisterArray(REGISTER_CONST2, const2.m_bytes, sizeof(const2.m_bytes));
    return const2.m_float;
}

uint8_t HeatControllerClass::readRegisterRaw(uint8_t r, uint8_t *dest, size_t len) {
    return static_cast<uint8_t>(readRegisterArray(r, dest, len));
}

uint8_t HeatControllerClass::writeRegisterRaw(uint8_t r, const uint8_t *src, size_t len) {
    return writeRegister(r, src, len);
}

size_t HeatControllerClass::registerCount(void) const {
    return sizeof(registers) / sizeof(registers[0]);
}

const RegisterInfo &HeatControllerClass::registerAt(size_t index) const {
    return registers[index];
}

uint8_t HeatControllerClass::registerSize(uint8_t r) const {
    for (size_t i = 0; i < registerCount(); i++) {
        if (registers[i].address == r) {
            return registers[i].size;
        }
    }

    return 0;
}

uint32_t HeatControllerClass::linkAgeMs(void) const {
    return millis() - lastHeartbeatMs;
}

bool HeatControllerClass::linkTimedOut(void) const {
    return linkTimedOutFlag;
}

bool HeatControllerClass::linkUp(void) const {
    return linkUpValue && !linkTimedOutFlag;
}

uint32_t HeatControllerClass::rewriteCount(void) const {
    return rewriteCountValue;
}

uint32_t HeatControllerClass::errorCount(void) const {
    return errorCountValue;
}

uint32_t HeatControllerClass::heartbeatCount(void) const {
    return heartbeatCountValue;
}

uint32_t HeatControllerClass::getLinkTimeout(void) {
    return readRegister_uint32_t(REGISTER_LINK_TIMEOUT);
}

uint8_t HeatControllerClass::setLinkTimeout(uint32_t ms) {
    return writeRegister(REGISTER_LINK_TIMEOUT, ms);
}

void HeatControllerClass::_INT() {
    this->heatInt = true;
}

uint8_t HeatControllerClass::selectRegister(uint8_t r) {
    Wire.beginTransmission(i2cAddress);
    Wire.write(r);
    return Wire.endTransmission();
}

size_t HeatControllerClass::readRegisterArray(uint8_t r, uint8_t *dest, size_t len) {
    if (selectRegister(r) != 0) {
        linkUpValue = false;
        return 0;
    }

    size_t got = Wire.requestFrom(i2cAddress, len);

    for (size_t b = 0; b < len; b++) {
        dest[b] = b < got ? static_cast<uint8_t>(Wire.read()) : 0;
    }

    if (got != len) {
        linkUpValue = false;
    }

    return got;
}

uint8_t HeatControllerClass::readRegister_uint8_t(uint8_t r) {
    uint8_t_u uint8_t_union;

    if (readRegisterArray(r, uint8_t_union.m_bytes, sizeof(uint8_t_union.m_bytes)) != sizeof(uint8_t_union.m_bytes)) {
        return 0;
    }

    return uint8_t_union.m_uint8_t;
}

uint32_t HeatControllerClass::readRegister_uint32_t(uint8_t r) {
    uint32_t_u uint32_t_union;

    if (readRegisterArray(r, uint32_t_union.m_bytes, sizeof(uint32_t_union.m_bytes)) != sizeof(uint32_t_union.m_bytes)) {
        return 0;
    }

    return uint32_t_union.m_uint32_t;
}

float HeatControllerClass::readRegister_float(uint8_t r) {
    float_u float_union;

    if (readRegisterArray(r, float_union.m_bytes, sizeof(float_union.m_bytes)) != sizeof(float_union.m_bytes)) {
        return NAN;
    }

    return float_union.m_float;
}

uint8_t HeatControllerClass::writeRegister(uint8_t r, const uint8_t *src, size_t len) {
    if (compatible == false) {
        return 1;
    }

    Wire.beginTransmission(i2cAddress);
    Wire.write(r);

    for (size_t b = len; b > 0; b--) {
        Wire.write(src[b - 1]);
    }

    return Wire.endTransmission();
}

uint8_t HeatControllerClass::writeRegister(uint8_t r, uint8_t v) {
    return writeRegister(r, &v, sizeof(v));
}

uint8_t HeatControllerClass::writeRegister(uint8_t r, uint16_t v) {
    uint16_t_u uint16_t_union;
    uint16_t_union.m_uint16_t = v;
    return writeRegister(r, uint16_t_union.m_bytes, sizeof(uint16_t_union.m_uint16_t));
}

uint8_t HeatControllerClass::writeRegister(uint8_t r, uint32_t v) {
    uint32_t_u uint32_t_union;
    uint32_t_union.m_uint32_t = v;
    return writeRegister(r, uint32_t_union.m_bytes, sizeof(uint32_t_union.m_uint32_t));
}

uint8_t HeatControllerClass::writeRegister(uint8_t r, uint64_t v) {
    uint64_t_u uint64_t_union;
    uint64_t_union.m_uint64_t = v;
    return writeRegister(r, uint64_t_union.m_bytes, sizeof(uint64_t_union.m_uint64_t));
}

uint8_t HeatControllerClass::writeRegister(uint8_t r, float v) {
    float_u float_union;
    float_union.m_float = v;
    return writeRegister(r, float_union.m_bytes, sizeof(float_union.m_float));
}

HeatControllerClass HeatController;
