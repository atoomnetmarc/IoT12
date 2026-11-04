/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "DeviceName.h"

#include <esp_mac.h>
#include <stdio.h>

namespace {
    char deviceName[32] = "iot12";
}

void DeviceNameSetup(void) {
    uint8_t mac[6];

    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    snprintf(deviceName, sizeof(deviceName), "iot12-%02x%02x%02x", mac[3], mac[4], mac[5]);
}

const char *DeviceNameGet(void) {
    return deviceName;
}
