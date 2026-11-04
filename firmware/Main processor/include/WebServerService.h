/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <stdint.h>

/*
 * Async web server, captive portal and live state stream.
 *
 * The server runs in both STA and AP mode. The portal is started and stopped
 * only on an explicit action from the touchscreen, the page or the CLI.
 */
namespace web_server {
    // Register the routes and start the server.
    void setup(void);

    // Service the captive DNS, deferred portal stop and the state stream.
    void loop(void);

    // Start the AP portal; pauses the station state machine.
    bool startPortal(void);

    // Stop the AP portal; resumes the station state machine.
    void stopPortal(void);

    // True while the AP portal is open.
    bool portalActive(void);

    // True while the server is running.
    bool running(void);

    // Request counter and last request URI, for the CLI.
    uint32_t requestCount(void);
    const char *lastUri(void);

    // AP identity, for the touchscreen and the CLI.
    const char *apSsid(void);
    const char *apPassword(void);
    const char *apIp(void);

    // Push the full state once per second to the SSE clients.
    void serviceStream(void);

    // Firmware upload state, for the CLI.
    bool otaActive(void);
    uint8_t otaProgress(void);
    int otaError(void);
}
