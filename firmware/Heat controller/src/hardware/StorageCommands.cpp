/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "StorageCommands.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "CliFormat.h"
#include "Storage.h"

namespace {
    constexpr uint32_t READ_CAP = 256;
    constexpr uint32_t CAT_CAP = 4096;

    const char *yesNo(bool value) {
        return value ? "yes" : "no";
    }

    /*
     * The exact protected range depends on the datasheet table for the fitted
     * device, so only the coarse shape is reported.
     */
    const char *protectionLabel(uint8_t bits) {
        if (bits == 0) {
            return "none";
        }

        if (bits == 4) {
            return "all";
        }

        return "partial";
    }

    const char *lockLabel(uint8_t bits) {
        if (bits == 0) {
            return "none";
        }

        if (bits == 7) {
            return "all";
        }

        return "partial";
    }

    const char *driveLabel(uint8_t bits) {
        static const char *const labels[] = {"100%", "75%", "50%", "25%"};

        return labels[bits & 0x03];
    }

    void flashInfo(cli::SerialCli &cli) {
        cli.writef("FLASH_JEDEC: 0x%06lX\r\n", static_cast<unsigned long>(Storage.jedecId()));
        cli_format::keyUInt(cli, "FLASH_SIZE", Storage.sizeBytes());

        uint8_t uid[8];

        if (Storage.uniqueId(uid)) {
            char text[sizeof(uid) * 2 + 1];
            size_t pos = 0;

            for (uint8_t value : uid) {
                pos += static_cast<size_t>(snprintf(text + pos, sizeof(text) - pos, "%02X", value));
            }

            cli.writef("FLASH_UID: %s\r\n", text);
        } else {
            cli.write("FLASH_UID: error\r\n");
        }

        uint8_t status1 = Storage.status1();
        uint8_t status2 = Storage.status2();
        uint8_t status3 = Storage.status3();

        cli.writef("FLASH_STATUS1: 0x%02X (busy=%s wel=%s bp=%u protected=%s tb=%s sec=%s srp=%s)\r\n",
                   status1,
                   yesNo((status1 & 0x01) != 0),
                   yesNo((status1 & 0x02) != 0),
                   static_cast<unsigned>((status1 >> 2) & 0x07),
                   protectionLabel(static_cast<uint8_t>((status1 >> 2) & 0x07)),
                   yesNo((status1 & 0x20) != 0),
                   yesNo((status1 & 0x40) != 0),
                   yesNo((status1 & 0x80) != 0));

        cli.writef("FLASH_STATUS2: 0x%02X (srl=%s qe=%s lock=%s cmp=%s sus=%s)\r\n",
                   status2,
                   yesNo((status2 & 0x01) != 0),
                   yesNo((status2 & 0x02) != 0),
                   lockLabel(static_cast<uint8_t>((status2 >> 3) & 0x07)),
                   yesNo((status2 & 0x40) != 0),
                   yesNo((status2 & 0x80) != 0));

        cli.writef("FLASH_STATUS3: 0x%02X (wps=%s drv=%s)\r\n",
                   status3,
                   yesNo((status3 & 0x04) != 0),
                   driveLabel(static_cast<uint8_t>((status3 >> 5) & 0x03)));

        cli_format::keyText(cli, "FLASH_READY", Storage.ready() ? "yes" : "no");
    }

    void flashRead(cli::SerialCli &cli, uint32_t address, uint32_t length) {
        if (length > READ_CAP) {
            length = READ_CAP;
        }

        uint8_t buffer[16];

        for (uint32_t offset = 0; offset < length; offset += sizeof(buffer)) {
            uint32_t chunk = length - offset;

            if (chunk > sizeof(buffer)) {
                chunk = sizeof(buffer);
            }

            if (!Storage.read(address + offset, buffer, chunk)) {
                cli.write("FLASH_READ: error\r\n");
                return;
            }

            char text[sizeof(buffer) * 3 + 1];
            size_t pos = 0;

            for (uint32_t i = 0; i < chunk; ++i) {
                pos += static_cast<size_t>(snprintf(text + pos, sizeof(text) - pos, "%02X ", buffer[i]));
            }

            cli.writef("FLASH_%08lX: %s\r\n", static_cast<unsigned long>(address + offset), text);
        }
    }

    void cmdFlash(cli::SerialCli &cli, int argc, char **argv) {
        if (argc >= 2 && strcmp(argv[1], "info") == 0) {
            flashInfo(cli);
            return;
        }

        if (argc >= 2 && strcmp(argv[1], "read") == 0) {
            if (argc < 4) {
                cli.write("Usage: flash read <addr> <len>\r\n");
                return;
            }

            flashRead(cli, strtoul(argv[2], nullptr, 0), strtoul(argv[3], nullptr, 0));
            return;
        }

        if (argc >= 2 && strcmp(argv[1], "erase") == 0) {
            if (argc < 3 || strcmp(argv[2], "yes-really") != 0) {
                cli.write("Usage: flash erase yes-really\r\n");
                return;
            }

            if (!Storage.eraseAll()) {
                cli.write("FLASH_ERASE: busy\r\n");
                return;
            }

            cli.write("FLASH_ERASE: started\r\n");
            return;
        }

        if (argc >= 2) {
            cli.write("Usage: flash [info|read <addr> <len>|erase yes-really]\r\n");
            return;
        }

        cli.writef("FLASH_JEDEC: 0x%06lX\r\n", static_cast<unsigned long>(Storage.jedecId()));
        cli_format::keyUInt(cli, "FLASH_SIZE", Storage.sizeBytes());
        cli_format::keyText(cli, "FLASH_READY", Storage.ready() ? "yes" : "no");
        cli_format::keyText(cli, "FLASH_ERASING", Storage.erasing() ? "yes" : "no");

        if (Storage.erasing()) {
            cli.writef("FLASH_ERASE_BLOCK: %lu/%lu\r\n",
                       static_cast<unsigned long>(Storage.eraseBlock()),
                       static_cast<unsigned long>(Storage.blockCount()));
        }
    }

    void fsList(cli::SerialCli &cli, const char *path) {
        if (!Storage.mounted()) {
            cli.write("FS_LS: not mounted\r\n");
            return;
        }

        lfs_dir_t dir;
        int err = lfs_dir_open(&Storage.fs(), &dir, path);

        if (err != LFS_ERR_OK) {
            cli.writef("FS_LS: error %d\r\n", err);
            return;
        }

        struct lfs_info info;

        while (lfs_dir_read(&Storage.fs(), &dir, &info) > 0) {
            if (strcmp(info.name, ".") == 0 || strcmp(info.name, "..") == 0) {
                continue;
            }

            cli.writef("FS_ENTRY: %c %lu %s\r\n",
                       info.type == LFS_TYPE_DIR ? 'd' : 'f',
                       static_cast<unsigned long>(info.size),
                       info.name);
        }

        lfs_dir_close(&Storage.fs(), &dir);
    }

    void fsCat(cli::SerialCli &cli, const char *path) {
        if (!Storage.mounted()) {
            cli.write("FS_CAT: not mounted\r\n");
            return;
        }

        lfs_file_t file;
        int err = lfs_file_open(&Storage.fs(), &file, path, LFS_O_RDONLY);

        if (err != LFS_ERR_OK) {
            cli.writef("FS_CAT: error %d\r\n", err);
            return;
        }

        cli.writef("FS_FILE: %s\r\n", path);

        char buffer[64];
        uint32_t total = 0;

        while (total < CAT_CAP) {
            lfs_ssize_t count = lfs_file_read(&Storage.fs(), &file, buffer, sizeof(buffer));

            if (count <= 0) {
                break;
            }

            cli.writef("%.*s", static_cast<int>(count), buffer);
            total += static_cast<uint32_t>(count);
        }

        lfs_file_close(&Storage.fs(), &file);

        cli.writef("\r\nFS_END: %lu\r\n", static_cast<unsigned long>(total));
    }

    void fsWrite(cli::SerialCli &cli, const char *path, const char *text) {
        if (!Storage.mounted()) {
            cli.write("FS_WRITE: not mounted\r\n");
            return;
        }

        lfs_file_t file;
        int err = lfs_file_open(&Storage.fs(), &file, path, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);

        if (err != LFS_ERR_OK) {
            cli.writef("FS_WRITE: error %d\r\n", err);
            return;
        }

        lfs_ssize_t written = lfs_file_write(&Storage.fs(), &file, text, strlen(text));
        lfs_file_close(&Storage.fs(), &file);

        if (written != static_cast<lfs_ssize_t>(strlen(text))) {
            cli.write("FS_WRITE: error\r\n");
            return;
        }

        cli.writef("FS_WRITE: %s %ld\r\n", path, static_cast<long>(written));
    }

    void fsRemove(cli::SerialCli &cli, const char *path) {
        if (!Storage.mounted()) {
            cli.write("FS_RM: not mounted\r\n");
            return;
        }

        int err = lfs_remove(&Storage.fs(), path);

        if (err != LFS_ERR_OK) {
            cli.writef("FS_RM: error %d\r\n", err);
            return;
        }

        cli.writef("FS_RM: %s\r\n", path);
    }

    void cmdFs(cli::SerialCli &cli, int argc, char **argv) {
        if (argc == 1) {
            cli_format::keyText(cli, "FS_MOUNTED", Storage.mounted() ? "yes" : "no");
            cli_format::keyUInt(cli, "FS_TOTAL", Storage.totalBytes());
            cli_format::keyUInt(cli, "FS_USED", Storage.usedBytes());
            return;
        }

        if (strcmp(argv[1], "ls") == 0) {
            fsList(cli, argc >= 3 ? argv[2] : "/");
            return;
        }

        if (strcmp(argv[1], "cat") == 0) {
            if (argc < 3) {
                cli.write("Usage: fs cat <path>\r\n");
                return;
            }

            fsCat(cli, argv[2]);
            return;
        }

        if (strcmp(argv[1], "write") == 0) {
            if (argc < 4) {
                cli.write("Usage: fs write <path> <text>\r\n");
                return;
            }

            fsWrite(cli, argv[2], argv[3]);
            return;
        }

        if (strcmp(argv[1], "rm") == 0) {
            if (argc < 4 || strcmp(argv[3], "yes-really") != 0) {
                cli.write("Usage: fs rm <path> yes-really\r\n");
                return;
            }

            fsRemove(cli, argv[2]);
            return;
        }

        if (strcmp(argv[1], "format") == 0) {
            if (argc < 3 || strcmp(argv[2], "yes-really") != 0) {
                cli.write("Usage: fs format yes-really\r\n");
                return;
            }

            cli.writef("FS_FORMAT: %s\r\n", Storage.format() ? "done" : "error");
            return;
        }

        cli.write("Usage: fs [ls [path]|cat <path>|write <path> <text>|rm <path> yes-really|format yes-really]\r\n");
    }
}

namespace storage_commands {
    void registerCommands(cli::SerialCli &cli) {
        cli.registerCommand({"flash", [&cli](int argc, char **argv) { cmdFlash(cli, argc, argv); }, "JEDEC ID, size, status; flash info; flash read <addr> <len>; flash erase yes-really"});
        cli.registerCommand({"fs", [&cli](int argc, char **argv) { cmdFs(cli, argc, argv); }, "mount state, size; fs ls [path]; fs cat <path>; fs write <path> <text>; fs rm <path> yes-really; fs format yes-really"});
    }
}
