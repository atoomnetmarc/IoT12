/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "NetCommands.h"

#include <WiFi.h>
#include <esp_ota_ops.h>

#include <cstring>

#include "CliFormat.h"
#include "DeviceName.h"
#include "HomeWiFiService.h"
#include "MdnsService.h"
#include "WebServerService.h"

namespace {
    void cmdWifi(cli::SerialCli &cli, int argc, char **argv) {
        (void)argc;
        (void)argv;

        cli_format::keyText(cli, "STATE", home_wifi::stateName(home_wifi::getState()));
        cli_format::keyText(cli, "SSID", WiFi.SSID().c_str());
        cli_format::keyText(cli, "IP", WiFi.localIP().toString().c_str());
        cli_format::keyText(cli, "HOSTNAME", DeviceNameGet());
        cli_format::keyInt(cli, "RSSI", WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0);
        cli_format::keyUInt(cli, "NETWORKS", home_wifi::networkCount());
        cli_format::keyUInt(cli, "CONNECTS", home_wifi::connectCount());
        cli_format::keyUInt(cli, "DISCONNECTS", home_wifi::disconnectCount());
    }

    void cmdWeb(cli::SerialCli &cli, int argc, char **argv) {
        (void)argc;
        (void)argv;

        cli_format::keyText(cli, "RUNNING", web_server::running() ? "yes" : "no");
        cli_format::keyText(cli, "PORTAL", web_server::portalActive() ? "on" : "off");
        cli_format::keyUInt(cli, "PORT", 80);
        cli_format::keyUInt(cli, "REQUESTS", web_server::requestCount());
        cli_format::keyText(cli, "LAST_URI", web_server::lastUri());
        cli_format::keyText(cli, "MDNS", mdns::running() ? "on" : "off");
        cli_format::keyText(cli, "AP_SSID", web_server::apSsid());
        cli_format::keyText(cli, "AP_IP", web_server::apIp());
    }

    void cmdOta(cli::SerialCli &cli, int argc, char **argv) {
        (void)argc;
        (void)argv;

        const esp_partition_t *running = esp_ota_get_running_partition();

        cli_format::keyText(cli, "STATE", web_server::otaActive() ? "uploading" : "idle");
        cli_format::keyUInt(cli, "PROGRESS", web_server::otaProgress());
        cli_format::keyInt(cli, "LAST_ERROR", web_server::otaError());

        if (running != nullptr) {
            cli_format::keyText(cli, "PARTITION", running->label);
            cli_format::keyUInt(cli, "SIZE", static_cast<unsigned long>(running->size));
        }
    }

    void cmdNet(cli::SerialCli &cli, int argc, char **argv) {
        (void)argc;
        (void)argv;

        cli_format::keyText(cli, "WIFI", home_wifi::stateName(home_wifi::getState()));
        cli_format::keyText(cli, "IP", WiFi.localIP().toString().c_str());
        cli_format::keyText(cli, "PORTAL", web_server::portalActive() ? "on" : "off");
        cli_format::keyUInt(cli, "REQUESTS", web_server::requestCount());
        cli_format::keyText(cli, "OTA", web_server::otaActive() ? "uploading" : "idle");
    }

    void cmdAp(cli::SerialCli &cli, int argc, char **argv) {
        if (argc < 2) {
            cli_format::keyText(cli, "AP", web_server::portalActive() ? "on" : "off");
            cli_format::keyText(cli, "SSID", web_server::apSsid());
            cli_format::keyText(cli, "PASS", web_server::apPassword());
            cli_format::keyText(cli, "IP", web_server::apIp());
            return;
        }

        if (strcmp(argv[1], "on") == 0) {
            bool ok = web_server::startPortal();
            cli_format::keyText(cli, "AP", ok ? "on" : "failed");
            return;
        }

        if (strcmp(argv[1], "off") == 0) {
            web_server::stopPortal();
            home_wifi::request();
            cli_format::keyText(cli, "AP", "off");
            return;
        }

        cli.write("Usage: ap [on|off]\r\n");
    }
}

namespace net_commands {
    void registerCommands(cli::SerialCli &cli) {
        cli.registerCommand({"wifi", [&cli](int argc, char **argv) { cmdWifi(cli, argc, argv); }, "station state, SSID, IP, RSSI, counters"});
        cli.registerCommand({"web", [&cli](int argc, char **argv) { cmdWeb(cli, argc, argv); }, "server state, portal, request count, last URI"});
        cli.registerCommand({"ota", [&cli](int argc, char **argv) { cmdOta(cli, argc, argv); }, "firmware upload state, progress, partition"});
        cli.registerCommand({"net", [&cli](int argc, char **argv) { cmdNet(cli, argc, argv); }, "combined WiFi, web and OTA summary"});
        cli.registerCommand({"ap", [&cli](int argc, char **argv) { cmdAp(cli, argc, argv); }, "AP portal state, SSID, password; ap on|off"});
    }
}
