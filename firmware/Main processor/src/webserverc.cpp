/*

Copyright 2021 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#include <WiFi.h>
//#include <DNSServer.h>
#include <ESPmDNS.h>

#include "webserverc.h"

WebServer HTTPServer(80);

void handleRoot()
{
    HTTPServer.send(200, "text/plain", "hello from esp!");
}

void handleNotFound()
{

    // Page not Found

    String message = "File Not Found\n\n";
    message.concat("URI: ");
    message.concat(HTTPServer.uri());
    message.concat("\nMethod: ");
    message.concat((HTTPServer.method() == HTTP_GET) ? "GET" : "POST");
    message.concat("\nArguments: ");
    message.concat(HTTPServer.args());
    message.concat("\n");
    for (uint8_t i = 0; i < HTTPServer.args(); i++)
    {
        message.concat(" " + HTTPServer.argName(i) + ": " + HTTPServer.arg(i) + "\n");
    }
    HTTPServer.send(404, "text/plain", message);
}

void WebservercSetup(void)
{
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("telnet", "tcp", 23);

    HTTPServer.on("/", handleRoot);

    HTTPServer.onNotFound(handleNotFound);

    HTTPServer.begin();

    Serial.println("HTTP server started");
}

void WebservercLoop(void)
{
    HTTPServer.handleClient();
}

void WebservercOnWiFiConnect(void)
{
    WebservercSetup();
}