/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include <WiFi.h>
// #include <DNSServer.h>
#include <ESPmDNS.h>

#include <cstring>

#include "Log.h"
#include "webserverc.h"

static const char *tag = "Web";

WebServer HTTPServer(80);

namespace {
    bool running = false;
    uint32_t requestCount = 0;
    char lastUri[32] = {0};

    void noteRequest(void) {
        requestCount++;

        String uri = HTTPServer.uri();
        size_t len = uri.length();
        if (len >= sizeof(lastUri)) {
            len = sizeof(lastUri) - 1;
        }

        memcpy(lastUri, uri.c_str(), len);
        lastUri[len] = '\0';
    }
}

void handleRoot() {
    noteRequest();
    HTTPServer.send(200, "text/plain", "hello from esp!");
}

void handleNotFound() {
    noteRequest();

    // Page not Found

    String message = "File Not Found\n\n";
    message.concat("URI: ");
    message.concat(HTTPServer.uri());
    message.concat("\nMethod: ");
    message.concat((HTTPServer.method() == HTTP_GET) ? "GET" : "POST");
    message.concat("\nArguments: ");
    message.concat(HTTPServer.args());
    message.concat("\n");
    for (uint8_t i = 0; i < HTTPServer.args(); i++) {
        message.concat(" " + HTTPServer.argName(i) + ": " + HTTPServer.arg(i) + "\n");
    }
    HTTPServer.send(404, "text/plain", message);
}

void WebservercSetup(void) {
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("telnet", "tcp", 23);

    HTTPServer.on("/", handleRoot);

    HTTPServer.onNotFound(handleNotFound);

    HTTPServer.begin();

    running = true;
    LOG_I(tag, "http server started on port 80");
}

void WebservercLoop(void) {
    HTTPServer.handleClient();
}

void WebservercOnWiFiConnect(void) {
    WebservercSetup();
}

bool WebservercRunning(void) {
    return running;
}

uint32_t WebservercRequestCount(void) {
    return requestCount;
}

const char *WebservercLastUri(void) {
    return lastUri;
}
