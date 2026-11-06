/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <stdint.h>

/*
 * Lifetime counters in NVS: the number of boots and the total uptime.
 *
 * The session uptime comes from esp_timer_get_time(), so it does not wrap at
 * 49.7 days like millis(). The total is written every 15 minutes, so a power
 * cut loses at most that interval.
 */
namespace lifetime_stats {
    // Load the counters, add one boot and write both back.
    void setup(void);

    // Write the total when the flush interval has passed.
    void loop(void);

    // Number of boots, including the current one.
    uint32_t bootCount(void);

    // Stored total plus the current session, in seconds.
    uint32_t totalSeconds(void);

    // Seconds since this boot.
    uint32_t sessionSeconds(void);

    // Write the total now.
    void flush(void);

    // Clear both counters and restart the session.
    void reset(void);
}
