/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "display.h"

#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <math.h>

#include "HeatController.h"

namespace {
    constexpr int16_t PIN_TFT_CS = 15;
    constexpr int16_t PIN_TFT_DC = 2;
    constexpr int16_t PIN_TFT_RST = 4;
    constexpr int16_t PIN_TFT_SCLK = 18;
    constexpr int16_t PIN_TFT_MISO = 19;
    constexpr int16_t PIN_TFT_MOSI = 23;
    constexpr int16_t PIN_TFT_LED = 32;
    constexpr int16_t PIN_TOUCH_CS = 5;
    constexpr int16_t PIN_TOUCH_IRQ = 34;

    constexpr uint8_t ROTATION = 1;
    constexpr uint8_t BACKLIGHT_DEFAULT = 255;
    constexpr uint32_t TEST_HOLD_MS = 3000;

    // ILI9341 panel with XPT2046 touch on one SPI bus, backlight on LEDC.
    class Panel : public lgfx::LGFX_Device {
    public:
        Panel(void) {
            {
                auto cfg = _bus.config();
                cfg.spi_host = SPI3_HOST;
                cfg.spi_mode = 0;
                cfg.freq_write = 40000000;
                cfg.freq_read = 16000000;
                cfg.spi_3wire = false;
                cfg.use_lock = true;
                cfg.dma_channel = SPI_DMA_CH_AUTO;
                cfg.pin_sclk = PIN_TFT_SCLK;
                cfg.pin_mosi = PIN_TFT_MOSI;
                cfg.pin_miso = PIN_TFT_MISO;
                cfg.pin_dc = PIN_TFT_DC;
                _bus.config(cfg);
                _panel.setBus(&_bus);
            }

            {
                auto cfg = _panel.config();
                cfg.pin_cs = PIN_TFT_CS;
                cfg.pin_rst = PIN_TFT_RST;
                cfg.pin_busy = -1;
                cfg.memory_width = 240;
                cfg.memory_height = 320;
                cfg.panel_width = 240;
                cfg.panel_height = 320;
                cfg.offset_rotation = 0;
                cfg.readable = true;
                cfg.invert = false;
                cfg.rgb_order = false;
                cfg.dlen_16bit = false;
                cfg.bus_shared = true;
                _panel.config(cfg);
            }

            {
                auto cfg = _light.config();
                cfg.pin_bl = PIN_TFT_LED;
                cfg.invert = false;
                cfg.freq = 5000;
                cfg.pwm_channel = 7;
                _light.config(cfg);
                _panel.setLight(&_light);
            }

            {
                auto cfg = _touch.config();
                cfg.x_min = 300;
                cfg.x_max = 3900;
                cfg.y_min = 300;
                cfg.y_max = 3900;
                cfg.pin_cs = PIN_TOUCH_CS;
                cfg.pin_int = PIN_TOUCH_IRQ;
                cfg.spi_host = SPI3_HOST;
                cfg.pin_sclk = PIN_TFT_SCLK;
                cfg.pin_mosi = PIN_TFT_MOSI;
                cfg.pin_miso = PIN_TFT_MISO;
                cfg.freq = 1000000;
                cfg.offset_rotation = 0;
                cfg.bus_shared = true;
                _touch.config(cfg);
                _panel.setTouch(&_touch);
            }

            setPanel(&_panel);
        }

    private:
        lgfx::Bus_SPI _bus;
        lgfx::Panel_ILI9341 _panel;
        lgfx::Light_PWM _light;
        lgfx::Touch_XPT2046 _touch;
    };

    Panel lcd;

    struct HomeState {
        int16_t tipTenths;
        int16_t setTenths;
        bool heaterOn;
        bool linked;
    };

    // Rectangle of a field that changes, used to clear only that field.
    struct FieldRect {
        int16_t x;
        int16_t y;
        int16_t w;
        int16_t h;
    };

    constexpr int16_t TIP_VALUE_Y = 40;
    constexpr int16_t SET_VALUE_X = 56;
    constexpr int16_t SET_VALUE_Y = 200;
    constexpr FieldRect HEATER_RECT = {188, 200, 128, 16};
    constexpr FieldRect LINK_RECT = {110, 150, 100, 16};

    bool drawRequested = true;
    uint32_t lastDrawMs = 0;
    uint32_t testUntilMs = 0;
    uint8_t backlight = BACKLIGHT_DEFAULT;
    bool powerOn = true;
    HomeState lastState = {INT16_MIN, INT16_MIN, false, false};

    // Read the values the home panel shows, rounded to the displayed precision.
    HomeState readHomeState(void) {
        HomeState state;

        state.linked = HeatController.isCompatible();
        state.tipTenths = static_cast<int16_t>(lroundf((HeatController.getTemperatureTip() - 273.15f) * 10.0f));
        state.setTenths = static_cast<int16_t>(lroundf((HeatController.getTargetTemperature() - 273.15f) * 10.0f));
        state.heaterOn = HeatController.getHeaterState() == HEATER_STATE::ON;

        return state;
    }

    // Clear a field rectangle to the background color.
    void clearField(const FieldRect &rect) {
        lcd.fillRect(rect.x, rect.y, rect.w, rect.h, TFT_BLACK);
    }

    // Draw the home panel: tip temperature, setpoint and heater state.
    // Only the fields that differ from the previous state are redrawn.
    void drawHome(const HomeState &state, const HomeState &previous, bool full) {
        if (full) {
            lcd.fillScreen(TFT_BLACK);
            lcd.setTextDatum(lgfx::textdatum_t::top_left);
            lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
            lcd.setTextSize(2);
            lcd.drawString("TIP", 8, 8);
            lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
            lcd.drawString("SET", 8, 200);
        }

        if (full || state.tipTenths != previous.tipTenths || state.linked != previous.linked) {
            char buffer[8];
            const char *text = "  --.-";

            if (state.linked) {
                snprintf(buffer, sizeof(buffer), "%6.1f", state.tipTenths / 10.0f);
                text = buffer;
            }
            lcd.setTextDatum(lgfx::textdatum_t::top_center);
            lcd.setTextColor(TFT_WHITE, TFT_BLACK);
            lcd.setTextSize(6);
            lcd.drawString(text, lcd.width() / 2, TIP_VALUE_Y);
        }

        if (full || state.setTenths != previous.setTenths || state.linked != previous.linked) {
            char buffer[8];
            const char *text = "  --.-";

            if (state.linked) {
                snprintf(buffer, sizeof(buffer), "%6.1f", state.setTenths / 10.0f);
                text = buffer;
            }
            lcd.setTextDatum(lgfx::textdatum_t::top_left);
            lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
            lcd.setTextSize(2);
            lcd.drawString(text, SET_VALUE_X, SET_VALUE_Y);
        }

        if (full || state.heaterOn != previous.heaterOn) {
            clearField(HEATER_RECT);
            lcd.setTextDatum(lgfx::textdatum_t::top_right);
            lcd.setTextColor(state.heaterOn ? TFT_GREEN : TFT_DARKGREY, TFT_BLACK);
            lcd.setTextSize(2);
            lcd.drawString(state.heaterOn ? "HEATER ON" : "HEATER OFF", lcd.width() - 8, HEATER_RECT.y);
        }

        if (full || state.linked != previous.linked) {
            clearField(LINK_RECT);
            if (!state.linked) {
                lcd.setTextDatum(lgfx::textdatum_t::top_center);
                lcd.setTextColor(TFT_RED, TFT_BLACK);
                lcd.setTextSize(2);
                lcd.drawString("NO LINK", lcd.width() / 2, LINK_RECT.y);
            }
        }
    }

    // Draw five color bars with a label in the middle.
    void drawTestPattern(void) {
        const uint16_t bars[] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_WHITE, TFT_BLACK};
        const int16_t barWidth = lcd.width() / 5;

        for (int16_t i = 0; i < 5; ++i) {
            lcd.fillRect(i * barWidth, 0, barWidth, lcd.height(), bars[i]);
        }

        lcd.setTextDatum(lgfx::textdatum_t::middle_center);
        lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
        lcd.setTextSize(3);
        lcd.drawString("DISPLAY TEST", lcd.width() / 2, lcd.height() / 2);
    }
}

void DisplaySetup(void) {
    lcd.init();
    lcd.setRotation(ROTATION);
    lcd.setBrightness(backlight);
    lcd.fillScreen(TFT_BLACK);
    drawRequested = true;
}

void DisplayLoop(void) {
    if (testUntilMs != 0) {
        if (static_cast<int32_t>(millis() - testUntilMs) < 0) {
            return;
        }
        testUntilMs = 0;
        drawRequested = true;
    }

    HomeState state = readHomeState();
    if (!drawRequested && state.tipTenths == lastState.tipTenths && state.setTenths == lastState.setTenths && state.heaterOn == lastState.heaterOn && state.linked == lastState.linked) {
        return;
    }

    drawHome(state, lastState, drawRequested);
    drawRequested = false;
    lastState = state;
    lastDrawMs = millis();
}

const char *DisplayDriverName(void) {
    return "ILI9341 (LovyanGFX)";
}

uint16_t DisplayWidth(void) {
    return lcd.width();
}

uint16_t DisplayHeight(void) {
    return lcd.height();
}

uint8_t DisplayRotation(void) {
    return lcd.getRotation();
}

uint8_t DisplayBacklight(void) {
    return backlight;
}

void DisplaySetBacklight(uint8_t level) {
    backlight = level;
    if (powerOn) {
        lcd.setBrightness(level);
    }
}

void DisplaySetPower(bool on) {
    powerOn = on;
    lcd.setBrightness(on ? backlight : 0);
}

void DisplayTestPattern(void) {
    drawTestPattern();
    testUntilMs = millis() + TEST_HOLD_MS;
}

uint32_t DisplayLastDrawMs(void) {
    return lastDrawMs;
}

bool TouchRead(uint16_t *x, uint16_t *y, uint16_t *rawX, uint16_t *rawY) {
    lgfx::touch_point_t raw;

    if (lcd.getTouchRaw(&raw, 1) == 0) {
        return false;
    }

    lgfx::touch_point_t mapped = raw;
    lcd.convertRawXY(&mapped, 1);

    if (x != nullptr) {
        *x = static_cast<uint16_t>(mapped.x);
    }
    if (y != nullptr) {
        *y = static_cast<uint16_t>(mapped.y);
    }
    if (rawX != nullptr) {
        *rawX = static_cast<uint16_t>(raw.x);
    }
    if (rawY != nullptr) {
        *rawY = static_cast<uint16_t>(raw.y);
    }

    return true;
}

bool TouchIrqLevel(void) {
    return digitalRead(PIN_TOUCH_IRQ) == HIGH;
}
