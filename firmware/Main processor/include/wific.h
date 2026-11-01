/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <stdint.h>

void WificSetup(void);
void WiFicLoop(void);

uint32_t WificConnectCount(void);
uint32_t WificDisconnectCount(void);
