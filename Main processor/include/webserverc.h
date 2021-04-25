/*

Copyright 2021 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#ifndef webserver_h
#define webserver_h

#include <WebServer.h>

extern WebServer HTTPServer;

void WebservercSetup(void);
void WebservercLoop(void);
void WebservercOnWiFiConnect(void);

#endif