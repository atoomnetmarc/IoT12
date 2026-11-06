/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

/*
 * mDNS responder for the station interface.
 *
 * The responder is bound to the lwIP netif that exists when it starts. A WiFi
 * mode change destroys and recreates that netif, so the responder must be
 * stopped before the mode changes and started again after the station
 * connects. Starting it at boot and leaving it running across a portal cycle
 * leaves a dangling netif pointer and crashes the device.
 */
namespace mdns {
    // Start advertising on the current station interface.
    void start(void);

    // Stop advertising; safe to call when not running.
    void stop(void);

    // True while the responder is advertising.
    bool running(void);
}
