/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <stdint.h>

enum OtaState : uint8_t {
    OTA_STATE_IDLE = 0,
    OTA_STATE_RUNNING,
    OTA_STATE_DONE,
    OTA_STATE_FAILED,
};

void OTASetup(void);
void OTALoop(void);

OtaState OtaGetState(void);
int OtaGetLastError(void);
uint8_t OtaGetProgress(void);
const char *OtaStateName(OtaState state);
