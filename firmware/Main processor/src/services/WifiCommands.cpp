/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "WifiCommands.h"

#include <WiFi.h>

#include "CliFormat.h"
#include "config.h"
#include "wific.h"

namespace {
    const char *statusName(wl_status_t status) {
        switch (status) {
            case WL_IDLE_STATUS:
                return "IDLE";
            case WL_NO_SSID_AVAIL:
                return "NO_SSID";
            case WL_SCAN_COMPLETED:
                return "SCAN_DONE";
            case WL_CONNECTED:
                return "CONNECTED";
            case WL_CONNECT_FAILED:
                return "CONNECT_FAILED";
            case WL_CONNECTION_LOST:
                return "CONNECTION_LOST";
            case WL_DISCONNECTED:
                return "DISCONNECTED";
            default:
                return "UNKNOWN";
        }
    }

    void cmdWifi(cli::SerialCli &cli, int argc, char **argv) {
        (void)argc;
        (void)argv;

        wl_status_t status = WiFi.status();

        cli_format::keyText(cli, "STATUS", statusName(status));
        cli_format::keyText(cli, "SSID", WiFi.SSID().c_str());
        cli_format::keyText(cli, "IP", WiFi.localIP().toString().c_str());
        cli_format::keyText(cli, "HOSTNAME", otahostname);
        cli_format::keyInt(cli, "RSSI", status == WL_CONNECTED ? WiFi.RSSI() : 0);
        cli_format::keyUInt(cli, "CONNECTS", WificConnectCount());
        cli_format::keyUInt(cli, "DISCONNECTS", WificDisconnectCount());
    }
}

namespace wifi_commands {
    void registerCommands(cli::SerialCli &cli) {
        cli.registerCommand({"wifi", [&cli](int argc, char **argv) { cmdWifi(cli, argc, argv); }, "status, SSID, IP, RSSI, connect counters"});
    }
}
