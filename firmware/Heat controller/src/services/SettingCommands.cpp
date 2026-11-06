/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "SettingCommands.h"

#include <cstdlib>
#include <cstring>

#include "CliFormat.h"
#include "Setting.h"

namespace {
    constexpr uint8_t DECIMALS = 6;

    void dumpSettings(cli::SerialCli &cli) {
        for (size_t i = 0; i < Setting.keyCount(); ++i) {
            const char *key = Setting.keyAt(i);
            float value = 0;

            Setting.get(key, value);
            cli_format::keyFloat(cli, key, value, DECIMALS);
        }
    }

    void getSetting(cli::SerialCli &cli, const char *key) {
        float value = 0;

        if (!Setting.get(key, value)) {
            cli.writef("settings: unknown key (%s)\r\n", key);
            return;
        }

        cli_format::keyFloat(cli, key, value, DECIMALS);
    }

    void setSetting(cli::SerialCli &cli, const char *key, const char *text) {
        char *end = nullptr;
        float value = strtof(text, &end);

        if (end == text || *end != '\0') {
            cli.writef("settings: not a number (%s)\r\n", text);
            return;
        }

        if (!Setting.set(key, value)) {
            cli.writef("settings: unknown key (%s)\r\n", key);
            return;
        }

        Setting.saveNow();

        float stored = 0;
        Setting.get(key, stored);
        cli_format::keyFloat(cli, key, stored, DECIMALS);
    }

    void fileInfo(cli::SerialCli &cli) {
        cli_format::keyText(cli, "SETTINGS_FILE", setting_files::PRIMARY);
        cli_format::keyUInt(cli, "SETTINGS_SIZE", Setting.fileSize(setting_files::PRIMARY));
        cli_format::keyText(cli, "SETTINGS_VALID", Setting.fileValid(setting_files::PRIMARY) ? "yes" : "no");
        cli_format::keyText(cli, "SETTINGS_BACKUP", setting_files::BACKUP);
        cli_format::keyUInt(cli, "SETTINGS_BACKUP_SIZE", Setting.fileSize(setting_files::BACKUP));
        cli_format::keyText(cli, "SETTINGS_BACKUP_VALID", Setting.fileValid(setting_files::BACKUP) ? "yes" : "no");
    }

    void cmdSettings(cli::SerialCli &cli, int argc, char **argv) {
        if (argc == 1) {
            dumpSettings(cli);
            return;
        }

        if (strcmp(argv[1], "get") == 0) {
            if (argc < 3) {
                cli.write("Usage: settings get <key>\r\n");
                return;
            }

            getSetting(cli, argv[2]);
            return;
        }

        if (strcmp(argv[1], "set") == 0) {
            if (argc < 4) {
                cli.write("Usage: settings set <key> <value>\r\n");
                return;
            }

            setSetting(cli, argv[2], argv[3]);
            return;
        }

        if (strcmp(argv[1], "save") == 0) {
            Setting.saveNow();
            cli.write("SETTINGS_SAVE: done\r\n");
            return;
        }

        if (strcmp(argv[1], "load") == 0) {
            Setting.load();
            cli.write("SETTINGS_LOAD: done\r\n");
            return;
        }

        if (strcmp(argv[1], "defaults") == 0) {
            if (argc < 3 || strcmp(argv[2], "yes-really") != 0) {
                cli.write("Usage: settings defaults yes-really\r\n");
                return;
            }

            Setting.defaults();
            Setting.saveNow();
            cli.write("SETTINGS_DEFAULTS: done\r\n");
            return;
        }

        if (strcmp(argv[1], "file") == 0) {
            fileInfo(cli);
            return;
        }

        cli.write("Usage: settings [get <key>|set <key> <value>|save|load|defaults yes-really|file]\r\n");
    }
}

namespace setting_commands {
    void registerCommands(cli::SerialCli &cli) {
        cli.registerCommand({"settings", [&cli](int argc, char **argv) { cmdSettings(cli, argc, argv); }, "dump all keys; settings get <key>; settings set <key> <value>; settings save; settings load; settings defaults yes-really; settings file"});
    }
}
