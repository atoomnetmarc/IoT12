/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include "SerialCli.h"

namespace system_commands {
    void registerCommands(cli::SerialCli &cli);

    /*
     * Set by `reboot yes-really`. The main loop performs the restart so the
     * handler itself never blocks and the reply has a chance to leave the UART.
     */
    bool rebootPending(void);
}
