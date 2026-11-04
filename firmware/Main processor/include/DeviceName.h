/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

// Build the device name "iot12-<mac>" from the STA MAC address.
void DeviceNameSetup(void);

// Device name, also used as the mDNS hostname and the AP SSID.
const char *DeviceNameGet(void);
