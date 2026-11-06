/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "MdnsService.h"

#include <ESPmDNS.h>

#include "DeviceName.h"
#include "Log.h"

static const char *tag = "mDNS";

namespace {
    bool active = false;
}

namespace mdns {
    void start(void) {
        if (active) {
            MDNS.end();
            active = false;
        }

        if (!MDNS.begin(DeviceNameGet())) {
            LOG_E(tag, "begin failed");
            return;
        }

        MDNS.addService("http", "tcp", 80);
        active = true;
        LOG_I(tag, "advertising %s.local", DeviceNameGet());
    }

    void stop(void) {
        if (!active) {
            return;
        }

        MDNS.end();
        active = false;
        LOG_I(tag, "stopped");
    }

    bool running(void) {
        return active;
    }
}
