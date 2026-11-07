/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "HeatCommands.h"

#include <cstdlib>
#include <cstring>

#include "CliFormat.h"
#include "HandleDiagnosis.h"
#include "HeatController.h"

namespace {
    void printFlags(cli::SerialCli &cli, uint32_t flags) {
        cli.writef("RAW: 0x%08lx\r\n", static_cast<unsigned long>(flags));
        cli_format::keyText(cli, "INT_REBOOT", (flags & (1UL << INT_REBOOT)) != 0 ? "1" : "0");
        cli_format::keyText(cli, "INT_MEASUREMENTS_GROUP_A", (flags & (1UL << INT_MEASUREMENTS_GROUP_A)) != 0 ? "1" : "0");
        cli_format::keyText(cli, "INT_MEASUREMENTS_GROUP_B", (flags & (1UL << INT_MEASUREMENTS_GROUP_B)) != 0 ? "1" : "0");
        cli_format::keyText(cli, "INT_LINK_TIMEOUT", (flags & (1UL << INT_LINK_TIMEOUT)) != 0 ? "1" : "0");
    }

    void printValue(cli::SerialCli &cli, const RegisterInfo &info, const uint8_t *bytes, uint8_t length) {
        if (length == 0) {
            cli.write("-\r\n");
            return;
        }

        if (strcmp(info.type, "str16") == 0) {
            char text[REGISTER_MAX_LENGTH + 1];
            memcpy(text, bytes, length);
            text[length] = '\0';
            cli.writef("%s\r\n", text);
            return;
        }

        if (strcmp(info.type, "u32x4") == 0) {
            uint32_t_u words[4];
            memcpy(words, bytes, sizeof(words));
            cli.writef("%08lx %08lx %08lx %08lx\r\n",
                       static_cast<unsigned long>(words[0].m_uint32_t),
                       static_cast<unsigned long>(words[1].m_uint32_t),
                       static_cast<unsigned long>(words[2].m_uint32_t),
                       static_cast<unsigned long>(words[3].m_uint32_t));
            return;
        }

        if (strcmp(info.type, "u32") == 0) {
            uint32_t_u value;
            memcpy(value.m_bytes, bytes, sizeof(value.m_bytes));
            cli.writef("0x%08lx\r\n", static_cast<unsigned long>(value.m_uint32_t));
            return;
        }

        if (strcmp(info.type, "u16") == 0) {
            uint16_t_u value;
            memcpy(value.m_bytes, bytes, sizeof(value.m_bytes));
            cli.writef("0x%04x\r\n", static_cast<unsigned>(value.m_uint16_t));
            return;
        }

        if (strcmp(info.type, "u8") == 0) {
            cli.writef("%u\r\n", static_cast<unsigned>(bytes[0]));
            return;
        }

        if (strcmp(info.type, "float") == 0) {
            float_u value;
            char buffer[24];
            memcpy(value.m_bytes, bytes, sizeof(value.m_bytes));
            cli_format::formatFloat(buffer, sizeof(buffer), value.m_float, 6);
            cli.writef("%s\r\n", buffer);
            return;
        }

        cli.write("-\r\n");
    }

    void cmdHeatInt(cli::SerialCli &cli) {
        printFlags(cli, HeatController.getInterrupt());
    }

    void cmdHeatRegs(cli::SerialCli &cli) {
        for (size_t i = 0; i < HeatController.registerCount(); i++) {
            const RegisterInfo &info = HeatController.registerAt(i);
            uint8_t bytes[REGISTER_MAX_LENGTH];
            uint8_t length = 0;

            if (info.address == REGISTER_INTERRUPT) {
                uint32_t_u value;
                value.m_uint32_t = HeatController.getLastInterrupt();
                memcpy(bytes, value.m_bytes, sizeof(value.m_bytes));
                length = sizeof(value.m_bytes);
            } else {
                length = HeatController.readRegisterRaw(info.address, bytes, sizeof(bytes));
            }

            cli.writef("REG: 0x%02x %-32s %-6s %-2s ",
                       static_cast<unsigned>(info.address),
                       info.name,
                       info.type,
                       info.writable ? "RW" : "R");
            printValue(cli, info, bytes, length);
        }
    }

    void cmdHeatRead(cli::SerialCli &cli, int argc, char **argv) {
        if (argc < 3) {
            cli.write("Usage: heat read <reg> [len]\r\n");
            return;
        }

        uint8_t reg = static_cast<uint8_t>(strtoul(argv[2], nullptr, 0));
        uint8_t bytes[REGISTER_MAX_LENGTH];
        uint8_t length = HeatController.registerSize(reg);

        if (argc >= 4) {
            length = static_cast<uint8_t>(strtoul(argv[3], nullptr, 0));
        }

        if (length == 0 || length > sizeof(bytes)) {
            length = sizeof(bytes);
        }

        length = HeatController.readRegisterRaw(reg, bytes, length);

        cli.writef("REGISTER: 0x%02x\r\n", reg);
        cli.writef("LENGTH: %u\r\n", static_cast<unsigned>(length));
        cli.write("BYTES:");

        for (uint8_t i = 0; i < length; i++) {
            cli.writef(" %02x", static_cast<unsigned>(bytes[i]));
        }

        cli.write("\r\n");
    }

    void cmdHeatWrite(cli::SerialCli &cli, int argc, char **argv) {
        if (argc < 5 || strcmp(argv[argc - 1], "yes-really") != 0) {
            cli.write("Usage: heat write <reg> <hex...> yes-really\r\n");
            return;
        }

        uint8_t reg = static_cast<uint8_t>(strtoul(argv[2], nullptr, 0));
        uint8_t bytes[REGISTER_MAX_LENGTH];
        uint8_t length = 0;

        for (int i = 3; i < argc - 1 && length < sizeof(bytes); i++) {
            bytes[length++] = static_cast<uint8_t>(strtoul(argv[i], nullptr, 16));
        }

        uint8_t result = HeatController.writeRegisterRaw(reg, bytes, length);
        cli.writef("WRITE: %s (%u)\r\n", result == 0 ? "ok" : "failed", static_cast<unsigned>(result));
    }

    void cmdHeatState(cli::SerialCli &cli, bool on) {
        if (!HeatController.isCompatible()) {
            cli.write("heat: not available (heat controller not compatible)\r\n");
            return;
        }

        uint8_t result = HeatController.setHeaterState(on ? HEATER_STATE::ON : HEATER_STATE::OFF);

        if (result != 0) {
            cli.writef("heat: write failed (%u)\r\n", result);
            return;
        }

        cli_format::keyText(cli, "STATE", on ? "ON" : "OFF");
    }

    void cmdHeatSetpoint(cli::SerialCli &cli, const char *argument) {
        if (!HeatController.isCompatible()) {
            cli.write("heat: not available (heat controller not compatible)\r\n");
            return;
        }

        float celsius = static_cast<float>(atof(argument));
        uint8_t result = HeatController.setTargetTemperature(celsius + 273.15f);

        if (result != 0) {
            cli.writef("heat: write failed (%u)\r\n", result);
            return;
        }

        cli_format::keyFloat(cli, "SETPOINT_C", celsius, 2);
    }

    void cmdSensor(cli::SerialCli &cli) {
        if (!HeatController.linkUp()) {
            cli.write("LINK: LOSS\r\n");
            return;
        }

        cli_format::keyFloat(cli, "TIP_TEMP_C", HeatController.getTemperatureTip() - 273.15f, 2);
        cli_format::keyFloat(cli, "NTC_TEMP_C", HeatController.getTemperatureNTC() - 273.15f, 2);
        cli_format::keyFloat(cli, "NTC_V", HeatController.getVoltageNTC(), 5);
        cli_format::keyFloat(cli, "THERMOCOUPLE_MV", HeatController.getVoltageThermocouple() * 1000.0f, 3);
        cli_format::keyFloat(cli, "VIN_OFF_V", HeatController.getSupplyVoltage(), 2);
        cli_format::keyFloat(cli, "VIN_ON_V", HeatController.getVoltageVinOn(), 2);
        cli_format::keyFloat(cli, "IRON_CURRENT_OFF_A", HeatController.getCurrentIronOff(), 3);
        cli_format::keyFloat(cli, "IRON_CURRENT_ON_A", HeatController.getCurrentIronOn(), 3);
        cli_format::keyFloat(cli, "RTIP_OHM", HeatController.getResistanceTip(), 2);
        cli_format::keyFloat(cli, "RSUPPLY_OHM", HeatController.getResistanceSupply(), 2);
        cli_format::keyText(cli, "HANDLE_STATE", HandleStateName(HandleDiagnosisClassify().state));
    }

    void cmdHeat(cli::SerialCli &cli, int argc, char **argv) {
        if (argc >= 2 && strcmp(argv[1], "int") == 0) {
            cmdHeatInt(cli);
            return;
        }

        if (argc >= 2 && strcmp(argv[1], "regs") == 0) {
            cmdHeatRegs(cli);
            return;
        }

        if (argc >= 2 && strcmp(argv[1], "read") == 0) {
            cmdHeatRead(cli, argc, argv);
            return;
        }

        if (argc >= 2 && strcmp(argv[1], "write") == 0) {
            cmdHeatWrite(cli, argc, argv);
            return;
        }

        if (argc >= 2 && strcmp(argv[1], "on") == 0) {
            cmdHeatState(cli, true);
            return;
        }

        if (argc >= 2 && strcmp(argv[1], "off") == 0) {
            cmdHeatState(cli, false);
            return;
        }

        if (argc >= 3 && strcmp(argv[1], "setpoint") == 0) {
            cmdHeatSetpoint(cli, argv[2]);
            return;
        }

        if (argc >= 2) {
            cli.write("Usage: heat [int|regs|read <reg> [len]|write <reg> <hex...> yes-really|on|off|setpoint <celsius>]\r\n");
            return;
        }

        SerialNumber_t serial = HeatController.getSerialNumber();

        cli_format::keyText(cli, "VERSION", HeatController.getFirmwareVersion());
        cli_format::keyText(cli, "COMPATIBLE", HeatController.isCompatible() ? "yes" : "no");
        cli_format::keyText(cli, "STATE", HeatController.getHeaterState() == HEATER_STATE::ON ? "ON" : "OFF");
        cli_format::keyFloat(cli, "TIP_C", HeatController.getTemperatureTip() - 273.15f, 2);
        cli_format::keyFloat(cli, "SETPOINT_C", HeatController.getTargetTemperature() - 273.15f, 2);
        cli_format::keyFloat(cli, "NTC_C", HeatController.getTemperatureNTC() - 273.15f, 2);
        cli.writef("SERIAL: %08lx %08lx %08lx %08lx\r\n",
                   static_cast<unsigned long>(serial.word0),
                   static_cast<unsigned long>(serial.word1),
                   static_cast<unsigned long>(serial.word2),
                   static_cast<unsigned long>(serial.word3));
        cli.writef("CONST1: 0x%04x\r\n", static_cast<unsigned>(HeatController.getConst1()));
        cli_format::keyFloat(cli, "CONST2", HeatController.getConst2(), 6);
        cli_format::keyUInt(cli, "LINK_AGE_MS", HeatController.linkAgeMs());
        cli_format::keyText(cli, "LINK_TIMEOUT", HeatController.linkTimedOut() ? "yes" : "no");
        cli_format::keyUInt(cli, "REWRITE_COUNT", HeatController.rewriteCount());
        cli_format::keyUInt(cli, "ERROR_COUNT", HeatController.errorCount());
        cli_format::keyUInt(cli, "HEARTBEAT_COUNT", HeatController.heartbeatCount());
        cli.writef("LAST_INT: 0x%08lx\r\n", static_cast<unsigned long>(HeatController.getLastInterrupt()));
    }

    void cmdLink(cli::SerialCli &cli, int argc, char **argv) {
        if (argc >= 2 && strcmp(argv[1], "timeout") == 0) {
            if (argc >= 3) {
                uint8_t result = HeatController.setLinkTimeout(static_cast<uint32_t>(strtoul(argv[2], nullptr, 0)));

                if (result != 0) {
                    cli.writef("link: write failed (%u)\r\n", static_cast<unsigned>(result));
                    return;
                }
            }

            cli_format::keyUInt(cli, "LINK_TIMEOUT_MS", HeatController.getLinkTimeout());
            return;
        }

        if (argc >= 2) {
            cli.write("Usage: link [timeout [<ms>]]\r\n");
            return;
        }

        cli_format::keyUInt(cli, "LINK_AGE_MS", HeatController.linkAgeMs());
        cli_format::keyUInt(cli, "LINK_TIMEOUT_MS", HeatController.getLinkTimeout());
        cli_format::keyText(cli, "LINK_TIMEOUT", HeatController.linkTimedOut() ? "yes" : "no");
        cli_format::keyUInt(cli, "REWRITE_COUNT", HeatController.rewriteCount());
        cli_format::keyUInt(cli, "ERROR_COUNT", HeatController.errorCount());
        cli_format::keyUInt(cli, "HEARTBEAT_COUNT", HeatController.heartbeatCount());
    }
}

namespace heat_commands {
    void registerCommands(cli::SerialCli &cli) {
        cli.registerCommand({"heat", [&cli](int argc, char **argv) { cmdHeat(cli, argc, argv); }, "heat controller: state, temperatures, link; heat int|regs|read|write|on|off|setpoint"});
        cli.registerCommand({"link", [&cli](int argc, char **argv) { cmdLink(cli, argc, argv); }, "link age, timeout flag, counters; link timeout [<ms>]"});
        cli.registerCommand({"sensor", [&cli](int argc, char **argv) { (void)argc; (void)argv; cmdSensor(cli); }, "handle measurements: temperatures, voltages, currents, resistances"});
    }
}
