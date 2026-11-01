/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <WebServer.h>

extern WebServer HTTPServer;

void WebservercSetup(void);
void WebservercLoop(void);
void WebservercOnWiFiConnect(void);

bool WebservercRunning(void);
uint32_t WebservercRequestCount(void);
const char *WebservercLastUri(void);
