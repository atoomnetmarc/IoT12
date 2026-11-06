/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

/*
 * NVS storage for the network array and the AP password.
 *
 * The network array is one JSON string under `wifi_cfg`:
 *   {"networks":[{"ssid":"...","pass":"..."},...]}
 * The AP password is a 12-character string under `ap_pass`.
 */
namespace web_config {
    constexpr const char *NVS_NAMESPACE = "iot12";
    constexpr const char *NVS_KEY_WIFI_CFG = "wifi_cfg";
    constexpr const char *NVS_KEY_AP_PASS = "ap_pass";

    constexpr size_t MAX_NETWORKS = 2;
    constexpr size_t SSID_LEN = 33;
    constexpr size_t PASS_LEN = 65;
    constexpr size_t AP_PASS_LEN = 13;

    struct Network {
        char ssid[SSID_LEN] = {};
        char pass[PASS_LEN] = {};
    };

    // Load the network array from NVS; returns the number of valid entries.
    size_t loadNetworks(Network *out, size_t maxCount);

    // Save a sanitized network array JSON string to NVS.
    bool saveNetworksJson(const char *json);

    // Parse network objects from a JSON array string.
    size_t parseNetworkArray(const char *json, Network *out, size_t maxCount);

    // Reduce a raw POST body to {"networks":[{"ssid":"...","pass":"..."}]}.
    int sanitizeNetworksBody(const char *rawBody, char *outJson, size_t outSize);

    // Load the AP password; returns false when it is not stored yet.
    bool loadApPassword(char *out, size_t outSize);

    // Store the AP password.
    bool saveApPassword(const char *pass);

    // Generate a 12-character AP password: 8 lowercase letters and 4 digits.
    void generateApPassword(char *out, size_t outSize);

    // Load the AP password, or generate and store a new one.
    bool ensureApPassword(char *out, size_t outSize);

    // Escape a string for use inside a JSON string literal.
    void escapeJson(const char *in, char *out, size_t outSize);
}
