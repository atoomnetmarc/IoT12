/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include <Arduino.h>
#include <Wire.h>
#include <wiring_private.h>

#include "EventLog.h"
#include "Heater.h"
#include "Setting.h"
#include "WireMinion.h"
#include "config.h"
#include "main.h"

namespace {
    const RegisterInfo registers[] = {
        {"VERSION", REGISTER_VERSION, "str16", FIRMWARE_VERSION_LENGTH, false},
        {"SERIAL_NUMBER", REGISTER_SERIAL_NUMBER, "u32x4", 16, false},
        {"INTERRUPT", REGISTER_INTERRUPT, "u32", 4, false},
        {"HEATER_STATE", REGISTER_HEATER_STATE, "u8", 1, true},
        {"LINK_TIMEOUT", REGISTER_LINK_TIMEOUT, "u32", 4, true},
        {"TEMPERATURE_HEATER", REGISTER_TEMPERATURE_HEATER, "float", 4, false},
        {"TEMPERATURE_NTC", REGISTER_TEMPERATURE_NTC, "float", 4, false},
        {"HEATER_DUTY", REGISTER_HEATER_DUTY, "float", 4, false},
        {"TEMPERATURE_HEATER_TARGET", REGISTER_TEMPERATURE_HEATER_TARGET, "float", 4, true},
        {"RESISTOR_NTC_PULLUP", REGISTER_RESISTOR_NTC_PULLUP, "float", 4, true},
        {"RESISTOR_HEATER_SHUNT", REGISTER_RESISTOR_HEATER_SHUNT, "float", 4, true},
        {"GAIN_VSUPPLY_RESISTOR_DIVIDER", REGISTER_GAIN_VSUPPLY_RESISTOR_DIVIDER, "float", 4, true},
        {"GAIN_HEATER_TEMPERATURE_AMPLIFIER", REGISTER_GAIN_HEATER_TEMPERATURE_AMPLIFIER, "float", 4, true},
        {"GAIN_HEATER_CURRENT_AMPLIFIER", REGISTER_GAIN_HEATER_CURRENT_AMPLIFIER, "float", 4, true},
        {"CONST1", REGISTER_CONST1, "u16", 2, false},
        {"CONST2", REGISTER_CONST2, "float", 4, false},
    };
}

void _requestEvent(void) {
    WireMinion.requestEvent();
}

void _receiveEvent(int numBytes) {
    WireMinion.receiveEvent(numBytes);
}

WireMinionClass::WireMinionClass() {
}

void WireMinionClass::init(void) {
    // Configure 2nd i2c port as a minion to our master controller.
    _minion.begin(I2C_MINION_ADDRESS);
    _minion.onRequest(_requestEvent);
    _minion.onReceive(_receiveEvent);
    pinPeripheral(2, PIO_SERCOM_ALT);
    pinPeripheral(3, PIO_SERCOM_ALT);

    pinMode(PIN_HEAT_INT, OUTPUT);

    _lastHeartbeatMs = millis();

    raiseInterrupt(INT_REBOOT);
}

void WireMinionClass::loop(void) {
    if (_assertHeatINT == true) {
        _assertHeatINT = false;
        digitalWrite(PIN_HEAT_INT, HIGH);
    }

    if (_linkTimedOut == false && (millis() - _lastHeartbeatMs) > _linkTimeoutMs) {
        _linkTimedOut = true;
        _forcedOffCount++;
        Setting.heaterState = HEATER_STATE::OFF;
        raiseInterrupt(INT_LINK_TIMEOUT);
        eventlog::Log.record(eventlog::LINK_TIMEOUT, static_cast<int32_t>(_linkTimeoutMs));
    }
}

void WireMinionClass::requestEvent(void) {
    uint8_t bytes[REGISTER_MAX_LENGTH];
    uint8_t sendBytes = readRegister(_i2cRegister, bytes, sizeof(bytes));

    // Only a read of the heater state is a link heartbeat.
    if (_i2cRegister == REGISTER_HEATER_STATE) {
        _lastHeartbeatMs = millis();
        _linkTimedOut = false;
    }

    _lastRegister = _i2cRegister;
    _lastWasRead = true;
    _lastLength = sendBytes;
    _lastTransactionMs = millis();

    for (uint8_t i = 0; i < sendBytes; i++) {
        _minion.write(bytes[i]);
    }
}

void WireMinionClass::receiveEvent(int numBytes) {
    (void)numBytes;

    // Read register address
    if (_minion.available()) {
        _i2cRegister = _minion.read();
    }

    // Read up to sizeof(_i2cData), ignore the rest.
    _i2cDataSize = 0;
    uint8_t downcounter = (sizeof(_i2cData) - 1);
    while (_minion.available()) {
        uint8_t b = _minion.read();

        if (downcounter > 0) {
            _i2cData[downcounter] = b;
            downcounter--;
            _i2cDataSize++;
        }
    }

    _lastRegister = _i2cRegister;
    _lastWasRead = false;
    _lastLength = _i2cDataSize;
    _lastTransactionMs = millis();

    // If not data received, do nothing.
    if (_i2cDataSize == 0) {
        return;
    }

    // Shift data to first element in array for easy processing.
    if (downcounter > 0) {
        for (uint8_t i = 0; i < _i2cDataSize; i++) {
            _i2cData[i] = _i2cData[i + downcounter + 1];
        }

        // Erase the remaining bytes for more easy debugging
        for (uint8_t i = _i2cDataSize; i < sizeof(_i2cData); i++) {
            _i2cData[i] = 0;
        }
    }

    writeRegister(_i2cRegister, _i2cData, _i2cDataSize);
}

uint8_t WireMinionClass::readRegister(uint8_t reg, uint8_t *dest, uint8_t maxLength) {
    uint8_t length = 0;
    float_u float_union;

    if (reg == REGISTER_INTERRUPT) {
        uint32_t_u uint32_t_union;
        uint32_t_union.m_uint32_t = _interruptFlags;
        length = sizeof(uint32_t_union.m_bytes);
        memcpy(dest, uint32_t_union.m_bytes, length);
        clearInterrupts();
    } else if (reg == REGISTER_SERIAL_NUMBER) {
        SerialNumber_u SerialNumber_union;
        SerialNumber_union.m_SerialNumber.word0 = *(uint32_t *)0x0080A00C;
        SerialNumber_union.m_SerialNumber.word1 = *(uint32_t *)0x0080A040;
        SerialNumber_union.m_SerialNumber.word2 = *(uint32_t *)0x0080A044;
        SerialNumber_union.m_SerialNumber.word3 = *(uint32_t *)0x0080A048;
        length = sizeof(SerialNumber_union.m_bytes);
        memcpy(dest, SerialNumber_union.m_bytes, length);
    } else if (reg == REGISTER_TEMPERATURE_HEATER_TARGET) {
        float_union.m_float = Setting.targetTemperatureHeater;
        length = sizeof(float_union.m_bytes);
        memcpy(dest, float_union.m_bytes, length);
    } else if (reg == REGISTER_TEMPERATURE_HEATER) {
        float_union.m_float = tTip;
        length = sizeof(float_union.m_bytes);
        memcpy(dest, float_union.m_bytes, length);
    } else if (reg == REGISTER_TEMPERATURE_NTC) {
        float_union.m_float = tAmbient;
        length = sizeof(float_union.m_bytes);
        memcpy(dest, float_union.m_bytes, length);
    } else if (reg == REGISTER_HEATER_DUTY) {
        // Actual PWM duty in percent, so the display can show the real output.
        HeaterStatus status = Heater.status();
        float_union.m_float = status.pwmPeriod > 0 ? (100.0f * status.pwmDuty / status.pwmPeriod) : 0.0f;
        length = sizeof(float_union.m_bytes);
        memcpy(dest, float_union.m_bytes, length);
    } else if (reg == REGISTER_VERSION) {
        length = FIRMWARE_VERSION_LENGTH;
        memset(dest, 0, length);
        strncpy((char *)dest, PRODUCT_VERSION, length - 1);
    } else if (reg == REGISTER_CONST1) {
        uint16_t_u uint16_t_union;
        uint16_t_union.m_uint16_t = 0x1234;
        length = sizeof(uint16_t_union.m_bytes);
        memcpy(dest, uint16_t_union.m_bytes, length);
    } else if (reg == REGISTER_CONST2) {
        float_union.m_float = PI;
        length = sizeof(float_union.m_bytes);
        memcpy(dest, float_union.m_bytes, length);
    } else if (reg == REGISTER_HEATER_STATE) {
        length = sizeof(HEATER_STATE);
        dest[0] = Setting.heaterState;
    } else if (reg == REGISTER_LINK_TIMEOUT) {
        uint32_t_u uint32_t_union;
        uint32_t_union.m_uint32_t = _linkTimeoutMs;
        length = sizeof(uint32_t_union.m_bytes);
        memcpy(dest, uint32_t_union.m_bytes, length);
    } else if (reg == REGISTER_GAIN_VSUPPLY_RESISTOR_DIVIDER) {
        float_union.m_float = Setting.gainVSupplyResistorDivider;
        length = sizeof(float_union.m_bytes);
        memcpy(dest, float_union.m_bytes, length);
    } else if (reg == REGISTER_GAIN_HEATER_TEMPERATURE_AMPLIFIER) {
        float_union.m_float = Setting.gainHeaterTemperatureAmplifier;
        length = sizeof(float_union.m_bytes);
        memcpy(dest, float_union.m_bytes, length);
    } else if (reg == REGISTER_GAIN_HEATER_CURRENT_AMPLIFIER) {
        float_union.m_float = Setting.gainHeaterCurrentAmplifier;
        length = sizeof(float_union.m_bytes);
        memcpy(dest, float_union.m_bytes, length);
    } else if (reg == REGISTER_RESISTOR_NTC_PULLUP) {
        float_union.m_float = Setting.resistanceNTCPullUp;
        length = sizeof(float_union.m_bytes);
        memcpy(dest, float_union.m_bytes, length);
    } else if (reg == REGISTER_RESISTOR_HEATER_SHUNT) {
        float_union.m_float = Setting.resistanceHeaterShunt;
        length = sizeof(float_union.m_bytes);
        memcpy(dest, float_union.m_bytes, length);
    }

    if (length > maxLength) {
        length = maxLength;
    }

    return length;
}

bool WireMinionClass::writeRegister(uint8_t reg, const uint8_t *data, uint8_t length) {
    float_u float_union;

    if (reg == REGISTER_HEATER_STATE && length == sizeof(HEATER_STATE)) {
        Setting.heaterState = (HEATER_STATE)data[0];
        return true;
    }

    if (reg == REGISTER_LINK_TIMEOUT && length == sizeof(uint32_t)) {
        uint32_t_u uint32_t_union;
        memcpy(uint32_t_union.m_bytes, data, sizeof(uint32_t_union.m_bytes));
        setLinkTimeout(uint32_t_union.m_uint32_t);
        return true;
    }

    if (reg == REGISTER_TEMPERATURE_HEATER_TARGET && length == sizeof(float_union.m_bytes)) {
        memcpy(float_union.m_bytes, data, sizeof(float_union.m_bytes));
        Setting.setTargetTemperatureHeater(float_union.m_float);
        Setting.save();
        return true;
    }

    if (reg == REGISTER_GAIN_VSUPPLY_RESISTOR_DIVIDER && length == sizeof(float_union.m_bytes)) {
        memcpy(float_union.m_bytes, data, sizeof(float_union.m_bytes));
        Setting.gainVSupplyResistorDivider = float_union.m_float;
        Setting.save();
        return true;
    }

    if (reg == REGISTER_GAIN_HEATER_TEMPERATURE_AMPLIFIER && length == sizeof(float_union.m_bytes)) {
        memcpy(float_union.m_bytes, data, sizeof(float_union.m_bytes));
        Setting.gainHeaterTemperatureAmplifier = float_union.m_float;
        Setting.save();
        return true;
    }

    if (reg == REGISTER_GAIN_HEATER_CURRENT_AMPLIFIER && length == sizeof(float_union.m_bytes)) {
        memcpy(float_union.m_bytes, data, sizeof(float_union.m_bytes));
        Setting.gainHeaterCurrentAmplifier = float_union.m_float;
        Setting.save();
        return true;
    }

    if (reg == REGISTER_RESISTOR_NTC_PULLUP && length == sizeof(float_union.m_bytes)) {
        memcpy(float_union.m_bytes, data, sizeof(float_union.m_bytes));
        Setting.resistanceNTCPullUp = float_union.m_float;
        Setting.save();
        return true;
    }

    if (reg == REGISTER_RESISTOR_HEATER_SHUNT && length == sizeof(float_union.m_bytes)) {
        memcpy(float_union.m_bytes, data, sizeof(float_union.m_bytes));
        Setting.resistanceHeaterShunt = float_union.m_float;
        Setting.save();
        return true;
    }

    return false;
}

void WireMinionClass::raiseInterrupt(uint8_t flag) {
    if (flag > 0 && flag < 32) {
        _interruptFlags |= (1UL << flag);
        _assertHeatINT = true;
    }
}

void WireMinionClass::clearInterrupts(void) {
    _interruptFlags = 0;
    _assertHeatINT = false;
    digitalWrite(PIN_HEAT_INT, LOW);
}

uint32_t WireMinionClass::interruptFlags(void) const {
    return _interruptFlags;
}

bool WireMinionClass::heatIntAsserted(void) const {
    return digitalRead(PIN_HEAT_INT) == HIGH;
}

size_t WireMinionClass::registerCount(void) const {
    return sizeof(registers) / sizeof(registers[0]);
}

const RegisterInfo &WireMinionClass::registerAt(size_t index) const {
    return registers[index];
}

void WireMinionClass::setLinkTimeout(uint32_t ms) {
    _linkTimeoutMs = ms < 100 ? 100 : ms;
}

uint32_t WireMinionClass::linkTimeout(void) const {
    return _linkTimeoutMs;
}

uint32_t WireMinionClass::lastHeartbeatAgeMs(void) const {
    return millis() - _lastHeartbeatMs;
}

bool WireMinionClass::linkTimedOut(void) const {
    return _linkTimedOut;
}

uint32_t WireMinionClass::forcedOffCount(void) const {
    return _forcedOffCount;
}

uint8_t WireMinionClass::lastRegister(void) const {
    return _lastRegister;
}

bool WireMinionClass::lastWasRead(void) const {
    return _lastWasRead;
}

uint8_t WireMinionClass::lastLength(void) const {
    return _lastLength;
}

uint32_t WireMinionClass::lastTransactionAgeMs(void) const {
    return millis() - _lastTransactionMs;
}

void WireMinionClass::onService(void) {
    _minion.onService();
}

WireMinionClass WireMinion;

void SERCOM2_Handler(void) {
    WireMinion.onService();
}
