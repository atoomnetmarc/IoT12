/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "Storage.h"

#include <Adafruit_SPIFlashBase.h>
#include <SPI.h>
#include <spi/Adafruit_FlashTransport_SPI.h>

#include "EventLog.h"
#include "config.h"

namespace {
    constexpr uint8_t CMD_READ_UNIQUE_ID = 0x4B;
    constexpr uint8_t CMD_READ_STATUS3 = 0x15;
    constexpr uint8_t UNIQUE_ID_DUMMY_BYTES = 4;
    constexpr uint32_t RAW_CLOCK = 4000000;

    constexpr uint32_t BLOCK_SIZE = SFLASH_SECTOR_SIZE;
    constexpr uint32_t READ_SIZE = 16;
    constexpr uint32_t PROG_SIZE = SFLASH_PAGE_SIZE;
    constexpr uint32_t CACHE_SIZE = PROG_SIZE;
    constexpr uint32_t LOOKAHEAD_SIZE = 16;
    constexpr uint32_t BLOCK_CYCLES = 500;

    Adafruit_FlashTransport_SPI flashTransport(PIN_FLASH_CS, SPI);
    Adafruit_SPIFlashBase flash(&flashTransport);

    lfs_t lfsVolume;
    struct lfs_config lfsConfig;

    uint8_t readBuffer[CACHE_SIZE];
    uint8_t progBuffer[CACHE_SIZE];
    uint8_t lookaheadBuffer[LOOKAHEAD_SIZE];

    int lfsRead(const struct lfs_config *config, lfs_block_t block, lfs_off_t offset, void *buffer, lfs_size_t size) {
        uint32_t address = block * config->block_size + offset;

        if (flash.readBuffer(address, static_cast<uint8_t *>(buffer), size) != size) {
            return LFS_ERR_IO;
        }

        return LFS_ERR_OK;
    }

    int lfsProg(const struct lfs_config *config, lfs_block_t block, lfs_off_t offset, const void *buffer, lfs_size_t size) {
        uint32_t address = block * config->block_size + offset;

        if (flash.writeBuffer(address, static_cast<const uint8_t *>(buffer), size) != size) {
            return LFS_ERR_IO;
        }

        return LFS_ERR_OK;
    }

    int lfsErase(const struct lfs_config *config, lfs_block_t block) {
        (void)config;

        if (!flash.eraseSector(block)) {
            return LFS_ERR_IO;
        }

        return LFS_ERR_OK;
    }

    int lfsSync(const struct lfs_config *config) {
        (void)config;

        return LFS_ERR_OK;
    }

    /*
     * Issue a command that is not part of the Adafruit transport: the transport
     * cannot insert dummy bytes, which the unique ID and status 3 commands need.
     */
    void rawRead(uint8_t command, uint8_t dummyBytes, uint8_t *buffer, uint32_t length) {
        SPI.beginTransaction(SPISettings(RAW_CLOCK, MSBFIRST, SPI_MODE0));
        digitalWrite(PIN_FLASH_CS, LOW);

        SPI.transfer(command);

        while (dummyBytes > 0) {
            SPI.transfer(0xFF);
            dummyBytes--;
        }

        for (uint32_t i = 0; i < length; ++i) {
            buffer[i] = SPI.transfer(0xFF);
        }

        digitalWrite(PIN_FLASH_CS, HIGH);
        SPI.endTransaction();
    }
}

StorageClass Storage;

bool StorageClass::init(void) {
    if (!flash.begin()) {
        eventlog::Log.record(eventlog::FLASH_ERROR, 0);
        return false;
    }

    _configure();

    _mounted = _mount();

    if (!_mounted) {
        eventlog::Log.record(eventlog::FS_ERROR, 0);
    }

    return _mounted;
}

void StorageClass::loop(void) {
    if (!_erasing) {
        return;
    }

    if (_eraseBlock >= blockCount()) {
        _erasing = false;
        return;
    }

    if (!flash.eraseSector(_eraseBlock)) {
        _erasing = false;
        eventlog::Log.record(eventlog::FLASH_ERROR, static_cast<int32_t>(_eraseBlock));
        return;
    }

    _eraseBlock++;
}

bool StorageClass::mounted(void) const {
    return _mounted;
}

bool StorageClass::erasing(void) const {
    return _erasing;
}

uint32_t StorageClass::jedecId(void) const {
    return flash.getJEDECID();
}

uint32_t StorageClass::sizeBytes(void) const {
    return flash.size();
}

uint32_t StorageClass::blockSize(void) const {
    return BLOCK_SIZE;
}

uint32_t StorageClass::blockCount(void) const {
    return lfsConfig.block_count;
}

uint32_t StorageClass::eraseBlock(void) const {
    return _eraseBlock;
}

uint32_t StorageClass::totalBytes(void) const {
    return blockCount() * BLOCK_SIZE;
}

uint32_t StorageClass::usedBytes(void) {
    if (!_mounted) {
        return 0;
    }

    lfs_ssize_t blocks = lfs_fs_size(&lfsVolume);

    if (blocks < 0) {
        return 0;
    }

    return static_cast<uint32_t>(blocks) * BLOCK_SIZE;
}

bool StorageClass::ready(void) {
    if (flash.size() == 0) {
        return false;
    }

    return flash.isReady();
}

bool StorageClass::read(uint32_t address, uint8_t *buffer, uint32_t length) {
    if (address > flash.size() || length > flash.size() - address) {
        return false;
    }

    return flash.readBuffer(address, buffer, length) == length;
}

bool StorageClass::uniqueId(uint8_t *buffer) {
    if (flash.size() == 0) {
        return false;
    }

    rawRead(CMD_READ_UNIQUE_ID, UNIQUE_ID_DUMMY_BYTES, buffer, 8);

    return true;
}

uint8_t StorageClass::status1(void) {
    return flash.readStatus();
}

uint8_t StorageClass::status2(void) {
    return flash.readStatus2();
}

uint8_t StorageClass::status3(void) {
    uint8_t value = 0;

    rawRead(CMD_READ_STATUS3, 0, &value, 1);

    return value;
}

bool StorageClass::format(void) {
    if (_erasing) {
        return false;
    }

    if (_mounted) {
        lfs_unmount(&lfsVolume);
    }

    _mounted = false;

    if (lfs_format(&lfsVolume, &lfsConfig) != LFS_ERR_OK) {
        eventlog::Log.record(eventlog::FS_ERROR, 0);
        return false;
    }

    _mounted = lfs_mount(&lfsVolume, &lfsConfig) == LFS_ERR_OK;

    if (!_mounted) {
        eventlog::Log.record(eventlog::FS_ERROR, 0);
    }

    return _mounted;
}

bool StorageClass::eraseAll(void) {
    if (_erasing) {
        return false;
    }

    if (_mounted) {
        lfs_unmount(&lfsVolume);
    }

    _mounted = false;
    _erasing = true;
    _eraseBlock = 0;

    return true;
}

lfs_t &StorageClass::fs(void) {
    return lfsVolume;
}

void StorageClass::_configure(void) {
    lfsConfig = {};

    lfsConfig.read = lfsRead;
    lfsConfig.prog = lfsProg;
    lfsConfig.erase = lfsErase;
    lfsConfig.sync = lfsSync;
    lfsConfig.read_size = READ_SIZE;
    lfsConfig.prog_size = PROG_SIZE;
    lfsConfig.block_size = BLOCK_SIZE;
    lfsConfig.block_count = flash.size() / BLOCK_SIZE;
    lfsConfig.block_cycles = BLOCK_CYCLES;
    lfsConfig.cache_size = CACHE_SIZE;
    lfsConfig.lookahead_size = LOOKAHEAD_SIZE;
    lfsConfig.read_buffer = readBuffer;
    lfsConfig.prog_buffer = progBuffer;
    lfsConfig.lookahead_buffer = lookaheadBuffer;
}

bool StorageClass::_mount(void) {
    if (_isBlank()) {
        lfs_format(&lfsVolume, &lfsConfig);
    }

    if (lfs_mount(&lfsVolume, &lfsConfig) == LFS_ERR_OK) {
        return true;
    }

    if (lfs_format(&lfsVolume, &lfsConfig) != LFS_ERR_OK) {
        return false;
    }

    return lfs_mount(&lfsVolume, &lfsConfig) == LFS_ERR_OK;
}

bool StorageClass::_isBlank(void) {
    uint8_t buffer[32];

    for (uint32_t offset = 0; offset < BLOCK_SIZE; offset += sizeof(buffer)) {
        if (flash.readBuffer(offset, buffer, sizeof(buffer)) != sizeof(buffer)) {
            return false;
        }

        for (uint8_t value : buffer) {
            if (value != 0xFF) {
                return false;
            }
        }
    }

    return true;
}
