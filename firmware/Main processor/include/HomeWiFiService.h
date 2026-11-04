/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#include "WebConfigStore.h"

/*
 * Station connection state machine.
 *
 * The service scans for the configured networks, connects to the strongest
 * match, and reconnects after a drop. The AP portal pauses the service, so
 * AP and STA never run at the same time.
 */
namespace home_wifi {
    enum class State : uint8_t {
        IDLE,
        SCANNING,
        CONNECTING,
        CONNECTED,
    };

    // Load the network array and start the state machine.
    void setup(void);

    // Advance the state machine; call from the application loop.
    void loop(void);

    // Request a connect attempt with the configured networks.
    void request(void);

    // Reload the network array from NVS and request a connect attempt.
    void reload(void);

    // Current state.
    State getState(void);

    // Name of a state, for the CLI and the page.
    const char *stateName(State state);

    // True when at least one network is configured.
    bool hasNetworks(void);

    // Configured networks as loaded from NVS.
    size_t networkCount(void);
    const web_config::Network *networks(void);

    // Suspend the state machine while the AP portal runs.
    void pause(void);

    // Resume the state machine after the portal stops.
    void resume(void);

    // True while the state machine is suspended.
    bool isSuppressed(void);

    // Connect and disconnect counters.
    uint32_t connectCount(void);
    uint32_t disconnectCount(void);
}
