/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "Ui.h"

#include <Arduino.h>
#include <math.h>
#include <stdio.h>

#include "HeatController.h"
#include "WebServerService.h"
#include "display.h"

namespace {
    constexpr int16_t SETPOINT_MIN_TENTHS = 0;    // 0.0 C
    constexpr int16_t SETPOINT_MAX_TENTHS = 4500; // 450.0 C
    constexpr int16_t SETPOINT_STEP_TENTHS = 50;  // 5.0 C

    constexpr uint32_t TOUCH_SAMPLE_MS = 20;
    constexpr uint32_t TOUCH_DEBOUNCE_MS = 30;
    constexpr uint32_t REPEAT_DELAY_MS = 500;
    constexpr uint32_t REPEAT_INTERVAL_MS = 150;

    struct ZoneInfo {
        UiZone zone;
        const char *name;
        UiRect rect;
    };

    const ZoneInfo zones[] = {
        {UI_ZONE_PREV, "PREV", UI_PREV_RECT},
        {UI_ZONE_NEXT, "NEXT", UI_NEXT_RECT},
        {UI_ZONE_POWER, "POWER", UI_POWER_RECT},
        {UI_ZONE_PLUS, "PLUS", UI_PLUS_RECT},
        {UI_ZONE_MINUS, "MINUS", UI_MINUS_RECT},
        {UI_ZONE_AP, "AP", UI_AP_RECT},
    };

    constexpr size_t ZONE_COUNT = sizeof(zones) / sizeof(zones[0]);
    constexpr UiRect EMPTY_RECT = {0, 0, 0, 0};

    UiZone pressedZone = UI_ZONE_NONE;
    UiZone activeZone = UI_ZONE_NONE;
    bool rawDown = false;
    bool stableDown = false;
    uint32_t rawChangeMs = 0;
    uint32_t lastSampleMs = 0;
    uint32_t pressStartMs = 0;
    uint32_t lastRepeatMs = 0;
    char lastEvent[24] = "none";

    int16_t clampSetpoint(int16_t tenths) {
        if (tenths < SETPOINT_MIN_TENTHS) {
            return SETPOINT_MIN_TENTHS;
        }
        if (tenths > SETPOINT_MAX_TENTHS) {
            return SETPOINT_MAX_TENTHS;
        }
        return tenths;
    }

    void stepSetpoint(int16_t deltaTenths) {
        int16_t current = UiSetpointTenths();
        int16_t next = clampSetpoint(current + deltaTenths);

        if (next == current) {
            return;
        }

        HeatController.setTargetTemperature(next / 10.0f + 273.15f);
        DisplayRequestDraw();
    }

    void toggleHeater(void) {
        HEATER_STATE next = UiHeaterOn() ? HEATER_STATE::OFF : HEATER_STATE::ON;

        HeatController.setHeaterState(next);
        DisplayRequestDraw();
    }

    void pressZone(UiZone zone) {
        switch (zone) {
            case UI_ZONE_PREV:
                DisplayScreenPrev();
                return;
            case UI_ZONE_NEXT:
                DisplayScreenNext();
                return;
            case UI_ZONE_AP:
                if (web_server::portalActive()) {
                    web_server::stopPortal();
                } else {
                    web_server::startPortal();
                }
                DisplayRequestDraw();
                return;
            default:
                break;
        }

        // Without a link the setpoint and the heater state are unknown, so a
        // touch must not write a value derived from a failed read.
        if (!HeatController.isCompatible()) {
            return;
        }

        activeZone = zone;

        switch (zone) {
            case UI_ZONE_POWER:
                toggleHeater();
                break;
            case UI_ZONE_PLUS:
                stepSetpoint(SETPOINT_STEP_TENTHS);
                break;
            case UI_ZONE_MINUS:
                stepSetpoint(-SETPOINT_STEP_TENTHS);
                break;
            default:
                break;
        }
    }

    void repeatZone(uint32_t now) {
        if (activeZone != UI_ZONE_PLUS && activeZone != UI_ZONE_MINUS) {
            return;
        }
        if (now - pressStartMs < REPEAT_DELAY_MS) {
            return;
        }
        if (now - lastRepeatMs < REPEAT_INTERVAL_MS) {
            return;
        }

        lastRepeatMs = now;
        stepSetpoint(activeZone == UI_ZONE_PLUS ? SETPOINT_STEP_TENTHS : -SETPOINT_STEP_TENTHS);
    }
}

void UiSetup(void) {
    pressedZone = UI_ZONE_NONE;
    activeZone = UI_ZONE_NONE;
    rawDown = false;
    stableDown = false;
    rawChangeMs = millis();
    lastSampleMs = millis();
    pressStartMs = millis();
    lastRepeatMs = millis();
    snprintf(lastEvent, sizeof(lastEvent), "none");
}

void UiLoop(void) {
    // The bezel box owns the panel while it is active.
    if (DisplayBoxActive()) {
        return;
    }

    uint32_t now = millis();

    if (now - lastSampleMs < TOUCH_SAMPLE_MS) {
        return;
    }
    lastSampleMs = now;

    bool down = false;
    uint16_t x = 0;
    uint16_t y = 0;

    if (!TouchIrqLevel()) {
        down = TouchRead(&x, &y, nullptr, nullptr);
    }

    if (down != rawDown) {
        rawDown = down;
        rawChangeMs = now;
    }

    if (rawDown != stableDown && now - rawChangeMs >= TOUCH_DEBOUNCE_MS) {
        stableDown = rawDown;

        if (stableDown) {
            pressedZone = UiHitTest(x, y);
            pressStartMs = now;
            lastRepeatMs = now;
            snprintf(lastEvent, sizeof(lastEvent), "press %s", UiZoneName(pressedZone));
            pressZone(pressedZone);
            DisplayRequestDraw();
        } else {
            pressedZone = UI_ZONE_NONE;
            activeZone = UI_ZONE_NONE;
            snprintf(lastEvent, sizeof(lastEvent), "release");
            DisplayRequestDraw();
        }
    }

    if (stableDown) {
        repeatZone(now);
    }
}

UiZone UiHitTest(uint16_t x, uint16_t y) {
    DisplayScreen screen = DisplayScreenGet();

    for (size_t i = 0; i < ZONE_COUNT; ++i) {
        UiZone zone = zones[i].zone;

        // The setpoint and heater zones belong to the home screen only.
        if ((zone == UI_ZONE_POWER || zone == UI_ZONE_PLUS || zone == UI_ZONE_MINUS) && screen != DISPLAY_SCREEN_HOME) {
            continue;
        }

        // The AP toggle belongs to the network screen only.
        if (zone == UI_ZONE_AP && screen != DISPLAY_SCREEN_NETWORK) {
            continue;
        }

        const UiRect &r = zones[i].rect;

        if (x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h) {
            return zone;
        }
    }

    return UI_ZONE_NONE;
}

UiZone UiPressedZone(void) {
    return pressedZone;
}

const char *UiZoneName(UiZone zone) {
    for (size_t i = 0; i < ZONE_COUNT; ++i) {
        if (zones[i].zone == zone) {
            return zones[i].name;
        }
    }

    return "NONE";
}

const UiRect &UiZoneRect(UiZone zone) {
    for (size_t i = 0; i < ZONE_COUNT; ++i) {
        if (zones[i].zone == zone) {
            return zones[i].rect;
        }
    }

    return EMPTY_RECT;
}

size_t UiZoneCount(void) {
    return ZONE_COUNT;
}

UiZone UiZoneAt(size_t index) {
    if (index >= ZONE_COUNT) {
        return UI_ZONE_NONE;
    }

    return zones[index].zone;
}

int16_t UiSetpointTenths(void) {
    return static_cast<int16_t>(lroundf((HeatController.getTargetTemperature() - 273.15f) * 10.0f));
}

bool UiHeaterOn(void) {
    return HeatController.getHeaterState() == HEATER_STATE::ON;
}

const char *UiLastEvent(void) {
    return lastEvent;
}

void UiSetSetpointTenths(int16_t tenths) {
    int16_t clamped = clampSetpoint(tenths);

    HeatController.setTargetTemperature(clamped / 10.0f + 273.15f);
    DisplayRequestDraw();
}

UiZone UiSyntheticPress(uint16_t x, uint16_t y) {
    UiZone zone = UiHitTest(x, y);

    snprintf(lastEvent, sizeof(lastEvent), "press %s", UiZoneName(zone));
    pressZone(zone);
    activeZone = UI_ZONE_NONE;
    DisplayRequestDraw();

    return zone;
}
