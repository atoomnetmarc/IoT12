/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <esp_system.h>

#include "SerialCli.h"

namespace system_commands {
    void registerCommands(cli::SerialCli &cli);

    // Name of an ESP32 reset reason, for the CLI and the web page.
    const char *resetReasonName(esp_reset_reason_t reason);

    /*
     * Set by `reboot yes-really`. The main loop performs the restart so the
     * handler itself never blocks and the reply has a chance to leave the UART.
     */
    bool rebootPending(void);
}
