/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "MinionCommands.h"

#include <cstdlib>
#include <cstring>

#include "CliFormat.h"
#include "WireMinion.h"

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

        if (strcmp(info.type, "u32x2") == 0) {
            uint32_t_u words[2];
            memcpy(words, bytes, sizeof(words));
            cli.writef("%lu %lu\r\n",
                       static_cast<unsigned long>(words[0].m_uint32_t),
                       static_cast<unsigned long>(words[1].m_uint32_t));
            return;
        }

        if (strcmp(info.type, "u8x4") == 0) {
            cli.writef("%u %u %u %u\r\n",
                       static_cast<unsigned>(bytes[0]),
                       static_cast<unsigned>(bytes[1]),
                       static_cast<unsigned>(bytes[2]),
                       static_cast<unsigned>(bytes[3]));
            return;
        }

        if (strcmp(info.type, "floatx4") == 0) {
            float_u values[4];
            char buffer[24];
            memcpy(values, bytes, sizeof(values));

            for (size_t i = 0; i < 4; i++) {
                cli_format::formatFloat(buffer, sizeof(buffer), values[i].m_float, 6);
                cli.writef("%s%s", i == 0 ? "" : " ", buffer);
            }

            cli.write("\r\n");
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

    void cmdMinionRead(cli::SerialCli &cli, int argc, char **argv) {
        if (argc < 3) {
            cli.write("Usage: minion read <reg> [len]\r\n");
            return;
        }

        uint8_t reg = static_cast<uint8_t>(strtoul(argv[2], nullptr, 0));
        uint8_t bytes[REGISTER_MAX_LENGTH];
        uint8_t length = WireMinion.readRegister(reg, bytes, sizeof(bytes));

        if (argc >= 4) {
            uint8_t requested = static_cast<uint8_t>(strtoul(argv[3], nullptr, 0));
            if (requested < length) {
                length = requested;
            }
        }

        cli.writef("REGISTER: 0x%02x\r\n", reg);
        cli.writef("LENGTH: %u\r\n", static_cast<unsigned>(length));
        cli.write("BYTES:");

        for (uint8_t i = 0; i < length; i++) {
            cli.writef(" %02x", static_cast<unsigned>(bytes[i]));
        }

        cli.write("\r\n");
    }

    void cmdMinionWrite(cli::SerialCli &cli, int argc, char **argv) {
        if (argc < 5 || strcmp(argv[argc - 1], "yes-really") != 0) {
            cli.write("Usage: minion write <reg> <hex...> yes-really\r\n");
            return;
        }

        uint8_t reg = static_cast<uint8_t>(strtoul(argv[2], nullptr, 0));
        uint8_t bytes[REGISTER_MAX_LENGTH];
        uint8_t length = 0;

        for (int i = 3; i < argc - 1 && length < sizeof(bytes); i++) {
            bytes[length++] = static_cast<uint8_t>(strtoul(argv[i], nullptr, 16));
        }

        cli.writef("WRITE: %s\r\n", WireMinion.writeRegister(reg, bytes, length) ? "ok" : "rejected");
    }

    void cmdMinion(cli::SerialCli &cli, int argc, char **argv) {
        if (argc >= 2 && strcmp(argv[1], "read") == 0) {
            cmdMinionRead(cli, argc, argv);
            return;
        }

        if (argc >= 2 && strcmp(argv[1], "write") == 0) {
            cmdMinionWrite(cli, argc, argv);
            return;
        }

        if (argc >= 2) {
            cli.write("Usage: minion [read <reg> [len]|write <reg> <hex...> yes-really]\r\n");
            return;
        }

        cli.writef("ADDRESS: 0x%02x\r\n", I2C_MINION_ADDRESS);
        cli.writef("LAST_REGISTER: 0x%02x\r\n", static_cast<unsigned>(WireMinion.lastRegister()));
        cli_format::keyText(cli, "LAST_DIRECTION", WireMinion.lastWasRead() ? "read" : "write");
        cli_format::keyUInt(cli, "LAST_LENGTH", WireMinion.lastLength());
        cli_format::keyUInt(cli, "LAST_AGE_MS", WireMinion.lastTransactionAgeMs());
        cli_format::keyText(cli, "HEAT_INT", WireMinion.heatIntAsserted() ? "high" : "low");
        printFlags(cli, WireMinion.interruptFlags());
    }

    void cmdRegs(cli::SerialCli &cli, int argc, char **argv) {
        (void)argc;
        (void)argv;

        for (size_t i = 0; i < WireMinion.registerCount(); i++) {
            const RegisterInfo &info = WireMinion.registerAt(i);
            uint8_t bytes[REGISTER_MAX_LENGTH];
            uint8_t length = 0;

            if (info.address == REGISTER_INTERRUPT) {
                uint32_t_u value;
                value.m_uint32_t = WireMinion.interruptFlags();
                memcpy(bytes, value.m_bytes, sizeof(value.m_bytes));
                length = sizeof(value.m_bytes);
            } else {
                length = WireMinion.readRegister(info.address, bytes, sizeof(bytes));
            }

            cli.writef("REG: 0x%02x %-32s %-6s %-2s ",
                       static_cast<unsigned>(info.address),
                       info.name,
                       info.type,
                       info.writable ? "RW" : "R");
            printValue(cli, info, bytes, length);
        }
    }

    void cmdInt(cli::SerialCli &cli, int argc, char **argv) {
        if (argc >= 2 && strcmp(argv[1], "clear") == 0) {
            WireMinion.clearInterrupts();
            cli.write("INT: cleared\r\n");
            return;
        }

        if (argc >= 2) {
            cli.write("Usage: int [clear]\r\n");
            return;
        }

        printFlags(cli, WireMinion.interruptFlags());
    }

    void cmdLink(cli::SerialCli &cli, int argc, char **argv) {
        if (argc >= 2 && strcmp(argv[1], "timeout") == 0) {
            if (argc >= 3) {
                WireMinion.setLinkTimeout(strtoul(argv[2], nullptr, 0));
            }

            cli_format::keyUInt(cli, "LINK_TIMEOUT_MS", WireMinion.linkTimeout());
            return;
        }

        if (argc >= 2) {
            cli.write("Usage: link [timeout [<ms>]]\r\n");
            return;
        }

        cli_format::keyUInt(cli, "HEARTBEAT_AGE_MS", WireMinion.lastHeartbeatAgeMs());
        cli_format::keyUInt(cli, "LINK_TIMEOUT_MS", WireMinion.linkTimeout());
        cli_format::keyText(cli, "LINK_TIMEOUT", WireMinion.linkTimedOut() ? "yes" : "no");
        cli_format::keyUInt(cli, "FORCED_OFF_COUNT", WireMinion.forcedOffCount());
    }
}

namespace minion_commands {
    void registerCommands(cli::SerialCli &cli) {
        cli.registerCommand({"minion", [&cli](int argc, char **argv) { cmdMinion(cli, argc, argv); }, "minion bus: address, last transaction, latched flags; minion read <reg> [len]; minion write <reg> <hex...> yes-really"});
        cli.registerCommand({"regs", [&cli](int argc, char **argv) { cmdRegs(cli, argc, argv); }, "full register map with names, addresses, types and current values"});
        cli.registerCommand({"int", [&cli](int argc, char **argv) { cmdInt(cli, argc, argv); }, "latched interrupt flags; int clear"});
        cli.registerCommand({"link", [&cli](int argc, char **argv) { cmdLink(cli, argc, argv); }, "heartbeat age, timeout flag, forced-off count; link timeout [<ms>]"});
    }
}
