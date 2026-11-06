/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <stdint.h>

#include "lfs.h"

/*
 * W25Q64 SPI flash and the LittleFS volume on it.
 */
class StorageClass {
public:
    bool init(void);
    void loop(void);

    bool mounted(void) const;
    bool erasing(void) const;

    uint32_t jedecId(void) const;
    uint32_t sizeBytes(void) const;
    uint32_t blockSize(void) const;
    uint32_t blockCount(void) const;
    uint32_t eraseBlock(void) const;

    uint32_t totalBytes(void) const;
    uint32_t usedBytes(void);

    bool ready(void);
    bool read(uint32_t address, uint8_t *buffer, uint32_t length);

    bool uniqueId(uint8_t *buffer);
    uint8_t status1(void);
    uint8_t status2(void);
    uint8_t status3(void);

    bool format(void);
    bool eraseAll(void);

    lfs_t &fs(void);

private:
    void _configure(void);
    bool _mount(void);
    bool _isBlank(void);

    bool _mounted = false;
    bool _erasing = false;
    uint32_t _eraseBlock = 0;
};

extern StorageClass Storage;
