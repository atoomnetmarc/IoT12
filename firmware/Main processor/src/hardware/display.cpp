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
#include <clib/u8g2.h>

#include "HeatController.h"
#include "Ui.h"

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
                // The touch panel's raw y axis runs opposite to the display x
                // axis, so swap the range to mirror the mapped x back.
                cfg.y_min = 3900;
                cfg.y_max = 300;
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
        int16_t tipTenths = 0;
        int16_t setTenths = 0;
        int16_t dutyPercent = 0;
        bool heaterOn = false;
        bool linked = false;
        UiZone pressedZone = UI_ZONE_NONE;
    };

    // Compare every displayed value, so DisplayLoop can skip an unchanged frame.
    bool operator==(const HomeState &a, const HomeState &b) {
        return a.tipTenths == b.tipTenths && a.setTenths == b.setTenths && a.dutyPercent == b.dutyPercent && a.heaterOn == b.heaterOn && a.linked == b.linked && a.pressedZone == b.pressedZone;
    }

    // Rectangle of a field that changes, used to clear only that field.
    struct FieldRect {
        int16_t x;
        int16_t y;
        int16_t w;
        int16_t h;
    };

    /*
     * Variant C geometry. The bezel hides 16 px on every edge, so every piece
     * of UI stays inside this inset. The top 124 px is the readout, the bottom
     * 84 px is the three-button strip. The button rects live in Ui.h, because
     * the touch hit-test uses the same numbers.
     */
    constexpr int16_t SAFE_MARGIN = 16;
    constexpr int16_t TIP_VALUE_X = 68;
    constexpr int16_t TIP_VALUE_Y = 56;
    constexpr int16_t SET_VALUE_X = 128;
    constexpr int16_t SET_VALUE_Y = 16;
    constexpr FieldRect TIP_VALUE_RECT = {16, 56, 288, 68};
    constexpr FieldRect SET_VALUE_RECT = {128, 16, 72, 35};
    constexpr FieldRect HEATER_RECT = {200, 16, 104, 35};

    constexpr uint16_t COLOR_BUTTON = 0x18E3;    // dark blue-grey
    constexpr uint16_t COLOR_SEPARATOR = 0x4208; // dark grey

    // drawRequested forces a full repaint (setup, leaving the font demo or the
    // bezel box). stateDirty only makes the loop re-read the values and redraw
    // the fields that changed, so a touch never wipes the whole panel.
    bool drawRequested = true;
    bool stateDirty = false;
    uint32_t lastDrawMs = 0;
    uint8_t backlight = BACKLIGHT_DEFAULT;
    bool powerOn = true;
    HomeState lastState;

    /*
     * The UI face is Logisoso, a U8g2 font, so it carries a real degree sign
     * (U+00B0), which no GFX font does. Two sizes cover the panel: 24 for the
     * labels, the setpoint and the buttons, 46 for the tip temperature.
     *
     * The u8g2_font_* arrays come from the U8g2 library (lib_deps). LovyanGFX
     * reads them directly through lgfx::U8g2font.
     */
    const lgfx::U8g2font fontLogisoso24(u8g2_font_logisoso24_tf);
    const lgfx::U8g2font fontLogisoso46(u8g2_font_logisoso46_tf);

    // Bezel measurement box. Stays on screen until it is stopped.
    bool boxActive = false;

    // Read the values the home panel shows, rounded to the displayed precision.
    HomeState readHomeState(void) {
        HomeState state;

        state.linked = HeatController.isCompatible();
        state.tipTenths = static_cast<int16_t>(lroundf((HeatController.getTemperatureTip() - 273.15f) * 10.0f));
        state.setTenths = static_cast<int16_t>(lroundf((HeatController.getTargetTemperature() - 273.15f) * 10.0f));
        state.dutyPercent = static_cast<int16_t>(lroundf(HeatController.getHeaterDuty()));
        state.heaterOn = HeatController.getHeaterState() == HEATER_STATE::ON;
        state.pressedZone = UiPressedZone();

        return state;
    }

    // Clear a field rectangle to the background color.
    void clearField(const FieldRect &rect) {
        lcd.fillRect(rect.x, rect.y, rect.w, rect.h, TFT_BLACK);
    }

    // Fill a button and draw its frame. A pressed button inverts and gets a
    // 2 px border, so the touch feedback is visible without a second color.
    void drawButtonFrame(const UiRect &rect, uint16_t fill, bool pressed) {
        lcd.fillRect(rect.x, rect.y, rect.w, rect.h, fill);
        lcd.drawRect(rect.x, rect.y, rect.w, rect.h, pressed ? TFT_WHITE : COLOR_SEPARATOR);
        if (pressed) {
            lcd.drawRect(rect.x + 1, rect.y + 1, rect.w - 2, rect.h - 2, TFT_WHITE);
        }
    }

    // POWER is green while the heater is on, dark grey while it is off.
    void drawPowerButton(bool heaterOn, bool pressed) {
        uint16_t fill = pressed ? TFT_WHITE : (heaterOn ? TFT_GREEN : COLOR_BUTTON);
        uint16_t text = (pressed || heaterOn) ? TFT_BLACK : TFT_WHITE;

        drawButtonFrame(UI_POWER_RECT, fill, pressed);

        lcd.setFont(&fontLogisoso24);
        lcd.setTextSize(1);
        lcd.setTextDatum(lgfx::textdatum_t::middle_center);
        lcd.setTextColor(text, fill);
        lcd.drawString("POWER", UI_POWER_RECT.x + UI_POWER_RECT.w / 2, UI_POWER_RECT.y + UI_POWER_RECT.h / 2);
    }

    // The step buttons draw their glyph as two bars, so they do not depend on
    // the font carrying a plus or minus.
    void drawStepButton(const UiRect &rect, bool plus, bool pressed) {
        constexpr int16_t ARM = 22;
        constexpr int16_t THICK = 6;

        uint16_t fill = pressed ? TFT_WHITE : COLOR_BUTTON;
        uint16_t glyph = pressed ? TFT_BLACK : TFT_WHITE;
        int16_t cx = rect.x + rect.w / 2;
        int16_t cy = rect.y + rect.h / 2;

        drawButtonFrame(rect, fill, pressed);

        lcd.fillRect(cx - ARM, cy - THICK / 2, 2 * ARM, THICK, glyph);
        if (plus) {
            lcd.fillRect(cx - THICK / 2, cy - ARM, THICK, 2 * ARM, glyph);
        }
    }

    // Draw the Variant C home panel: readout on top, three buttons below.
    // Only the fields that differ from the previous state are redrawn.
    void drawHome(const HomeState &state, const HomeState &previous, bool full) {
        if (full) {
            lcd.fillScreen(TFT_BLACK);

            lcd.setFont(&fontLogisoso24);
            lcd.setTextSize(1);
            lcd.setTextDatum(lgfx::textdatum_t::top_left);
            lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
            lcd.drawString("TIP", SAFE_MARGIN, SAFE_MARGIN);
            lcd.drawString("SET", 72, SAFE_MARGIN);
        }

        /*
         * The text is drawn with a background color instead of clearing the
         * field first. LovyanGFX paints the glyph cells, so a fixed-width
         * string overwrites the previous one without a black flash. The field
         * is only cleared when the format changes (link up or down).
         */
        if (full || state.tipTenths != previous.tipTenths || state.linked != previous.linked) {
            char buffer[16];

            if (state.linked != previous.linked) {
                clearField(TIP_VALUE_RECT);
            }

            if (state.linked) {
                // %5.1f keeps three integer digits, so the value never moves.
                snprintf(buffer, sizeof(buffer), "%5.1f\xC2\xB0" "C", state.tipTenths / 10.0f);
            } else {
                snprintf(buffer, sizeof(buffer), "--.-");
            }

            lcd.setFont(&fontLogisoso46);
            lcd.setTextSize(1);
            lcd.setTextDatum(lgfx::textdatum_t::top_left);
            lcd.setTextColor(state.linked ? TFT_WHITE : TFT_DARKGREY, TFT_BLACK);
            lcd.drawString(buffer, TIP_VALUE_X, TIP_VALUE_Y);
        }

        if (full || state.setTenths != previous.setTenths || state.linked != previous.linked) {
            char buffer[16];

            if (state.linked != previous.linked) {
                clearField(SET_VALUE_RECT);
            }

            if (state.linked) {
                snprintf(buffer, sizeof(buffer), "%5.1f", state.setTenths / 10.0f);
            } else {
                snprintf(buffer, sizeof(buffer), "--.-");
            }

            lcd.setFont(&fontLogisoso24);
            lcd.setTextSize(1);
            lcd.setTextDatum(lgfx::textdatum_t::top_left);
            lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
            lcd.drawString(buffer, SET_VALUE_X, SET_VALUE_Y);
        }

        if (full || state.heaterOn != previous.heaterOn || state.dutyPercent != previous.dutyPercent || state.linked != previous.linked) {
            char buffer[16];
            const char *text = "    OFF";
            uint16_t color = TFT_DARKGREY;

            if (!state.linked) {
                text = "NO LINK";
                color = TFT_RED;
            } else if (state.heaterOn) {
                // Pad to seven characters so the background band covers the
                // previous value; the extra characters are blank spaces.
                snprintf(buffer, sizeof(buffer), "   %3d%%", state.dutyPercent);
                text = buffer;
                color = TFT_GREEN;
            }

            if (state.linked != previous.linked) {
                clearField(HEATER_RECT);
            }

            lcd.setFont(&fontLogisoso24);
            lcd.setTextSize(1);
            lcd.setTextDatum(lgfx::textdatum_t::top_right);
            lcd.setTextColor(color, TFT_BLACK);
            lcd.drawString(text, lcd.width() - SAFE_MARGIN, SAFE_MARGIN);
        }

        /*
         * Redraw each button only when its own appearance changed. Repainting
         * the whole strip on every press made the other two buttons blink.
         */
        bool powerPressed = state.pressedZone == UI_ZONE_POWER;
        bool plusPressed = state.pressedZone == UI_ZONE_PLUS;
        bool minusPressed = state.pressedZone == UI_ZONE_MINUS;
        bool powerPressedBefore = previous.pressedZone == UI_ZONE_POWER;
        bool plusPressedBefore = previous.pressedZone == UI_ZONE_PLUS;
        bool minusPressedBefore = previous.pressedZone == UI_ZONE_MINUS;

        if (full || state.heaterOn != previous.heaterOn || powerPressed != powerPressedBefore) {
            drawPowerButton(state.heaterOn, powerPressed);
        }
        if (full || plusPressed != plusPressedBefore) {
            drawStepButton(UI_PLUS_RECT, true, plusPressed);
        }
        if (full || minusPressed != minusPressedBefore) {
            drawStepButton(UI_MINUS_RECT, false, minusPressed);
        }
    }

    // Draw the panel edge and a rectangle inset by margin pixels.
    void drawBox(uint16_t margin) {
        const int16_t w = lcd.width();
        const int16_t h = lcd.height();
        const int16_t m = static_cast<int16_t>(margin);

        lcd.fillScreen(TFT_BLACK);

        // Panel edge, so the inset is visible.
        lcd.drawRect(0, 0, w, h, TFT_DARKGREY);

        if (m > 0 && (m * 2) < w && (m * 2) < h) {
            lcd.drawRect(m, m, w - 2 * m, h - 2 * m, TFT_WHITE);
            lcd.drawRect(m + 1, m + 1, w - 2 * m - 2, h - 2 * m - 2, TFT_WHITE);
        }

        char text[16];
        snprintf(text, sizeof(text), "%u px", margin);

        lcd.setFont(&fonts::DejaVu24);
        lcd.setTextSize(1);
        lcd.setTextDatum(lgfx::textdatum_t::middle_center);
        lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
        lcd.drawString(text, w / 2, h / 2);
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
    if (boxActive) {
        return;
    }

    HomeState state = readHomeState();
    if (!drawRequested && !stateDirty && state == lastState) {
        return;
    }

    drawHome(state, lastState, drawRequested);
    drawRequested = false;
    stateDirty = false;
    lastState = state;
    lastDrawMs = millis();
}

void DisplayRequestDraw(void) {
    // Only re-evaluate the fields. A full repaint would flash the panel.
    stateDirty = true;
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

void DisplayDrawBox(uint16_t margin) {
    boxActive = true;
    drawBox(margin);
}

void DisplayBoxStop(void) {
    boxActive = false;
    drawRequested = true;
}

bool DisplayBoxActive(void) {
    return boxActive;
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
