/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "display.h"

#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <WiFi.h>
#include <clib/u8g2.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <math.h>
#include <string.h>

#include "HeatController.h"
#include "HomeWiFiService.h"
#include "LifetimeStats.h"
#include "Ui.h"
#include "WebServerService.h"

#ifndef PRODUCT_VERSION
#define PRODUCT_VERSION "unknown"
#endif

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
     * Geometry. The bezel hides 16 px on every edge, so every piece of UI
     * stays inside this inset. The top 32 px is the carousel header, the
     * readout sits below it, and the bottom 84 px is the button strip.
     */
    constexpr int16_t SAFE_MARGIN = 16;
    constexpr int16_t HEADER_Y = 16;
    constexpr int16_t HEADER_H = 32;
    constexpr int16_t TITLE_CENTER_X = 160;

    constexpr FieldRect TIP_VALUE_RECT = {16, 56, 288, 56};
    constexpr FieldRect SET_VALUE_RECT = {75, 120, 75, 30};
    constexpr FieldRect HEATER_RECT = {213, 120, 91, 30};

    // Width of the label column on the Network and Info screens, and the height
    // of one value row. The link label switches between AP and STA, so the
    // column is cleared when the label changes. The widest label is UPTIME at
    // 89 px, which ends at x=105.
    constexpr int16_t LABEL_COLUMN_W = 64;
    constexpr int16_t VALUE_H = 30;

    constexpr uint16_t COLOR_BUTTON = 0x18E3;    // dark blue-grey
    constexpr uint16_t COLOR_SEPARATOR = 0x4208; // dark grey

    // drawRequested forces a full repaint (setup, screen change or the bezel
    // box). stateDirty only makes the loop re-read the values and redraw the
    // fields that changed, so a touch never wipes the whole panel.
    bool drawRequested = true;
    bool stateDirty = false;
    uint32_t lastDrawMs = 0;
    uint8_t backlight = BACKLIGHT_DEFAULT;
    bool powerOn = true;
    HomeState lastState;
    UiZone lastHeaderPressed = UI_ZONE_NONE;
    DisplayScreen currentScreen = DISPLAY_SCREEN_HOME;

    /*
     * The panel is shared between the main loop (drawing) and the AsyncTCP
     * task (screenshot reads). LovyanGFX serializes individual SPI
     * transactions, but not the LGFX object state, so the whole draw and the
     * whole capture take this mutex. A null mutex means the allocation failed;
     * the display then runs without the lock instead of staying blank.
     */
    SemaphoreHandle_t displayMutex = nullptr;

    bool lockDisplay(uint32_t timeoutMs) {
        if (displayMutex == nullptr) {
            return true;
        }
        return xSemaphoreTake(displayMutex, pdMS_TO_TICKS(timeoutMs)) == pdTRUE;
    }

    void unlockDisplay(void) {
        if (displayMutex != nullptr) {
            xSemaphoreGive(displayMutex);
        }
    }

    // RAII guard, so every early return in DisplayLoop releases the panel.
    struct DisplayLock {
        bool held;

        explicit DisplayLock(uint32_t timeoutMs) : held(lockDisplay(timeoutMs)) {
        }

        ~DisplayLock() {
            if (held) {
                unlockDisplay();
            }
        }
    };

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

    /*
     * Draw a right-aligned value. Only the cells that a wider previous value
     * left behind are cleared, so a value that changes every second does not
     * blink and the clear never reaches the label on the same row. A null
     * previous means the row is already blank.
     */
    void drawValue(const char *value, const char *previous, int16_t y, uint16_t color) {
        lcd.setFont(&fontLogisoso24);
        lcd.setTextSize(1);
        lcd.setTextDatum(lgfx::textdatum_t::middle_right);
        lcd.setTextColor(color, TFT_BLACK);

        int16_t right = lcd.width() - SAFE_MARGIN;
        int16_t left = right - lcd.textWidth(value);

        if (previous != nullptr) {
            int16_t previousLeft = right - lcd.textWidth(previous);

            if (previousLeft < left) {
                lcd.fillRect(previousLeft, y, left - previousLeft, VALUE_H, TFT_BLACK);
            }
        }

        lcd.drawString(value, right, y + 12);
    }

    // Draw a label on the left and a right-aligned value on the same row. The
    // value goes first, so its clear never eats into the label.
    void drawLabelValue(const char *label, const char *value, const char *previous, int16_t y, uint16_t valueColor) {
        drawValue(value, previous, y, valueColor);

        lcd.setFont(&fontLogisoso24);
        lcd.setTextSize(1);
        lcd.setTextDatum(lgfx::textdatum_t::top_left);
        lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
        lcd.drawString(label, SAFE_MARGIN, y);
    }

    // Draw the AP toggle button. It is the same size and place as the middle
    // home button, so the two screens share one button geometry.
    void drawApButton(const UiRect &rect, bool ap, bool pressed) {
        uint16_t fill = pressed ? TFT_WHITE : (ap ? TFT_GREEN : COLOR_BUTTON);
        uint16_t text = (pressed || ap) ? TFT_BLACK : TFT_WHITE;

        drawButtonFrame(rect, fill, pressed);

        lcd.setFont(&fontLogisoso24);
        lcd.setTextSize(1);
        lcd.setTextDatum(lgfx::textdatum_t::middle_center);
        lcd.setTextColor(text, fill);
        lcd.drawString("AP", rect.x + rect.w / 2, rect.y + rect.h / 2);
    }

    // Draw one carousel arrow box. The chevron is drawn as two lines, so it
    // does not depend on the font carrying an arrow glyph.
    void drawArrowBox(const UiRect &rect, bool left, bool pressed) {
        uint16_t fill = pressed ? TFT_WHITE : COLOR_BUTTON;
        uint16_t glyph = pressed ? TFT_BLACK : TFT_WHITE;
        int16_t cx = rect.x + rect.w / 2;
        int16_t cy = rect.y + rect.h / 2;
        int16_t dx = left ? 5 : -5;

        drawButtonFrame(rect, fill, pressed);

        lcd.drawLine(cx + dx, cy - 8, cx - dx, cy, glyph);
        lcd.drawLine(cx - dx, cy, cx + dx, cy + 8, glyph);
    }

    // Draw the carousel header: two arrow boxes and the centered title. An
    // arrow is only repainted when its pressed state changes, so a periodic
    // field update never makes the header blink.
    void drawHeader(bool full) {
        UiZone pressed = UiPressedZone();

        if (full) {
            lcd.fillRect(0, 0, lcd.width(), HEADER_Y + HEADER_H + 4, TFT_BLACK);
        }

        if (full || pressed == UI_ZONE_PREV || lastHeaderPressed == UI_ZONE_PREV) {
            drawArrowBox(UI_PREV_RECT, true, pressed == UI_ZONE_PREV);
        }
        if (full || pressed == UI_ZONE_NEXT || lastHeaderPressed == UI_ZONE_NEXT) {
            drawArrowBox(UI_NEXT_RECT, false, pressed == UI_ZONE_NEXT);
        }

        if (full) {
            lcd.setFont(&fontLogisoso24);
            lcd.setTextSize(1);
            lcd.setTextDatum(lgfx::textdatum_t::middle_center);
            lcd.setTextColor(TFT_WHITE, TFT_BLACK);
            lcd.drawString(DisplayScreenName(currentScreen), TITLE_CENTER_X, HEADER_Y + HEADER_H / 2);
        }

        lastHeaderPressed = pressed;
    }

    // POWER is green while the heater is on, dark grey while it is off. A
    // layout variant can keep the button dark and show the state with a dot.
    void drawPowerButtonAt(const UiRect &rect, bool heaterOn, bool pressed, bool fillWhenOn) {
        bool lit = heaterOn && fillWhenOn;
        uint16_t fill = pressed ? TFT_WHITE : (lit ? TFT_GREEN : COLOR_BUTTON);
        uint16_t text = (pressed || lit) ? TFT_BLACK : TFT_WHITE;

        drawButtonFrame(rect, fill, pressed);

        lcd.setFont(&fontLogisoso24);
        lcd.setTextSize(1);
        lcd.setTextDatum(lgfx::textdatum_t::middle_center);
        lcd.setTextColor(text, fill);
        lcd.drawString("POWER", rect.x + rect.w / 2, rect.y + rect.h / 2);
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

    // Draw the big tip value at a fixed anchor. The caller clears the field
    // when the link state changes, because the string width then changes.
    void drawTipField(const HomeState &state, int16_t x, int16_t y, lgfx::textdatum_t datum) {
        char buffer[16];

        if (state.linked) {
            snprintf(buffer, sizeof(buffer), "%5.1f\xC2\xB0"
                                             "C",
                     state.tipTenths / 10.0f);
        } else {
            snprintf(buffer, sizeof(buffer), "--.-");
        }

        lcd.setFont(&fontLogisoso46);
        lcd.setTextSize(1);
        lcd.setTextDatum(datum);
        lcd.setTextColor(state.linked ? TFT_WHITE : TFT_DARKGREY, TFT_BLACK);
        lcd.drawString(buffer, x, y);
    }

    void drawSetField(const HomeState &state, int16_t x, int16_t y, lgfx::textdatum_t datum) {
        char buffer[16];

        if (state.linked) {
            snprintf(buffer, sizeof(buffer), "%5.1f", state.setTenths / 10.0f);
        } else {
            snprintf(buffer, sizeof(buffer), "--.-");
        }

        lcd.setFont(&fontLogisoso24);
        lcd.setTextSize(1);
        lcd.setTextDatum(datum);
        lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        lcd.drawString(buffer, x, y);
    }

    void drawHeaterField(const HomeState &state, int16_t x, int16_t y, lgfx::textdatum_t datum) {
        char buffer[16];
        // Four characters wide, so OFF, LOST and the duty share one cell width
        // and the right-aligned value never reaches the PWM label.
        const char *text = " OFF";
        uint16_t color = TFT_DARKGREY;

        if (!state.linked) {
            text = "LOST";
            color = TFT_RED;
        } else if (state.heaterOn) {
            snprintf(buffer, sizeof(buffer), "%3d%%", state.dutyPercent);
            text = buffer;
            color = TFT_GREEN;
        }

        lcd.setFont(&fontLogisoso24);
        lcd.setTextSize(1);
        lcd.setTextDatum(datum);
        lcd.setTextColor(color, TFT_BLACK);
        lcd.drawString(text, x, y);
    }

    // Redraw only the buttons whose appearance changed, for any layout.
    void drawHomeButtons(const HomeState &state, const HomeState &previous, bool full, const UiRect &power, const UiRect &plus, const UiRect &minus, bool fillPowerWhenOn) {
        bool powerPressed = state.pressedZone == UI_ZONE_POWER;
        bool plusPressed = state.pressedZone == UI_ZONE_PLUS;
        bool minusPressed = state.pressedZone == UI_ZONE_MINUS;
        bool powerPressedBefore = previous.pressedZone == UI_ZONE_POWER;
        bool plusPressedBefore = previous.pressedZone == UI_ZONE_PLUS;
        bool minusPressedBefore = previous.pressedZone == UI_ZONE_MINUS;

        if (full || state.heaterOn != previous.heaterOn || powerPressed != powerPressedBefore) {
            drawPowerButtonAt(power, state.heaterOn, powerPressed, fillPowerWhenOn);
        }
        if (full || plusPressed != plusPressedBefore) {
            drawStepButton(plus, true, plusPressed);
        }
        if (full || minusPressed != minusPressedBefore) {
            drawStepButton(minus, false, minusPressed);
        }
    }

    // Draw the home panel: header, hero value, status rail and three buttons.
    // Only the fields that differ from the previous state are redrawn.
    void drawHome(const HomeState &state, const HomeState &previous, bool full) {
        if (full) {
            lcd.fillScreen(TFT_BLACK);
        }

        drawHeader(full);

        if (full) {
            lcd.drawFastHLine(SAFE_MARGIN, 116, lcd.width() - 2 * SAFE_MARGIN, COLOR_SEPARATOR);

            lcd.setFont(&fontLogisoso24);
            lcd.setTextSize(1);
            lcd.setTextDatum(lgfx::textdatum_t::top_left);
            lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
            lcd.drawString("SET", SAFE_MARGIN, 120);
            lcd.drawString("PWM", 160, 120);
        }

        /*
         * The text is drawn with a background color instead of clearing the
         * field first. LovyanGFX paints the glyph cells, so a fixed-width
         * string overwrites the previous one without a black flash. The field
         * is only cleared when the format changes (link up or down).
         */
        if (full || state.tipTenths != previous.tipTenths || state.linked != previous.linked) {
            if (state.linked != previous.linked) {
                clearField(TIP_VALUE_RECT);
            }
            drawTipField(state, 160, 56, lgfx::textdatum_t::top_center);
        }

        if (full || state.setTenths != previous.setTenths || state.linked != previous.linked) {
            if (state.linked != previous.linked) {
                clearField(SET_VALUE_RECT);
            }
            // Right-aligned, so the value grows to the left and never touches
            // the SET label when it crosses from two to three digits.
            drawSetField(state, 150, 120, lgfx::textdatum_t::top_right);
        }

        if (full || state.heaterOn != previous.heaterOn || state.dutyPercent != previous.dutyPercent || state.linked != previous.linked) {
            if (state.linked != previous.linked) {
                clearField(HEATER_RECT);
            }
            drawHeaterField(state, 304, 120, lgfx::textdatum_t::top_right);
        }

        /*
         * Redraw each button only when its own appearance changed. Repainting
         * the whole strip on every press made the other two buttons blink.
         */
        drawHomeButtons(state, previous, full, UI_POWER_RECT, UI_PLUS_RECT, UI_MINUS_RECT, true);
    }

    // Values the network screen shows.
    struct NetworkState {
        bool ap = false;
        char ssid[33] = "";
        char pass[17] = "";
        char linkLabel[4] = "";
        char linkValue[16] = "";
        UiZone pressedZone = UI_ZONE_NONE;
    };

    // Compare every displayed value, so DisplayLoop can skip an unchanged frame.
    bool operator==(const NetworkState &a, const NetworkState &b) {
        return a.ap == b.ap && strcmp(a.ssid, b.ssid) == 0 && strcmp(a.pass, b.pass) == 0 && strcmp(a.linkLabel, b.linkLabel) == 0 && strcmp(a.linkValue, b.linkValue) == 0 && a.pressedZone == b.pressedZone;
    }

    NetworkState lastNetworkState;

    /*
     * Read the network values. The third row shows where the device is
     * reachable: the AP address while the portal is open, the station address
     * once the station is connected, and the state name while it is not.
     */
    NetworkState readNetworkState(void) {
        NetworkState state;

        state.ap = web_server::portalActive();
        snprintf(state.ssid, sizeof(state.ssid), "%s", web_server::apSsid());
        snprintf(state.pass, sizeof(state.pass), "%s", web_server::apPassword());
        state.pressedZone = UiPressedZone();

        if (state.ap) {
            snprintf(state.linkLabel, sizeof(state.linkLabel), "AP");
            snprintf(state.linkValue, sizeof(state.linkValue), "%s", web_server::apIp());
        } else {
            snprintf(state.linkLabel, sizeof(state.linkLabel), "STA");

            if (home_wifi::getState() == home_wifi::State::CONNECTED) {
                snprintf(state.linkValue, sizeof(state.linkValue), "%s", WiFi.localIP().toString().c_str());
            } else {
                snprintf(state.linkValue, sizeof(state.linkValue), "%s", home_wifi::stateName(home_wifi::getState()));
            }
        }

        return state;
    }

    /*
     * Draw the network screen. The SSID, the WPA2 password and the link row
     * take the three rows, and the AP button sits in the middle slot of the
     * button strip. The button colour carries the AP state, so no separate
     * status row is needed. Only the rows whose value changed are redrawn.
     */
    void drawNetwork(const NetworkState &state, const NetworkState &previous, bool full) {
        if (full) {
            lcd.fillScreen(TFT_BLACK);
        }

        drawHeader(full);

        if (full) {
            lcd.drawFastHLine(SAFE_MARGIN, 52, lcd.width() - 2 * SAFE_MARGIN, COLOR_SEPARATOR);
        }

        if (full || strcmp(state.ssid, previous.ssid) != 0) {
            drawLabelValue("SSID", state.ssid, full ? nullptr : previous.ssid, 56, TFT_WHITE);
        }

        if (full || strcmp(state.pass, previous.pass) != 0) {
            drawLabelValue("PASS", state.pass, full ? nullptr : previous.pass, 92, TFT_WHITE);
        }

        if (full || strcmp(state.linkLabel, previous.linkLabel) != 0 || strcmp(state.linkValue, previous.linkValue) != 0) {
            // The label switches between AP and STA, so clear the label column
            // first; a shorter label would otherwise leave a glyph behind.
            if (strcmp(state.linkLabel, previous.linkLabel) != 0) {
                lcd.fillRect(SAFE_MARGIN, 128, LABEL_COLUMN_W, VALUE_H, TFT_BLACK);
            }
            drawLabelValue(state.linkLabel, state.linkValue, full ? nullptr : previous.linkValue, 128, TFT_WHITE);
        }

        if (full || state.ap != previous.ap || state.pressedZone != previous.pressedZone) {
            drawApButton(UI_AP_RECT, state.ap, state.pressedZone == UI_ZONE_AP);
        }
    }

    // Values the info screen shows.
    struct InfoState {
        char version[16] = "";
        uint32_t session = 0;
        uint32_t total = 0;
        uint32_t boots = 0;
    };

    // Compare every displayed value, so DisplayLoop can skip an unchanged frame.
    bool operator==(const InfoState &a, const InfoState &b) {
        return strcmp(a.version, b.version) == 0 && a.session == b.session && a.total == b.total && a.boots == b.boots;
    }

    InfoState lastInfoState;

    // Format a duration as "27h 14m", or "14m 03s" below one hour.
    void formatDuration(uint32_t seconds, char *buffer, size_t size) {
        uint32_t hours = seconds / 3600U;
        uint32_t minutes = (seconds / 60U) % 60U;

        if (hours > 0) {
            snprintf(buffer, size, "%luh %02lum", static_cast<unsigned long>(hours), static_cast<unsigned long>(minutes));
        } else {
            snprintf(buffer, size, "%lum %02lus", static_cast<unsigned long>(minutes), static_cast<unsigned long>(seconds % 60U));
        }
    }

    // Read the info values.
    InfoState readInfoState(void) {
        InfoState state;

        snprintf(state.version, sizeof(state.version), "%s", PRODUCT_VERSION);
        state.session = lifetime_stats::sessionSeconds();
        state.total = lifetime_stats::totalSeconds();
        state.boots = lifetime_stats::bootCount();

        return state;
    }

    // Draw the info screen. Only the rows whose value changed are redrawn.
    void drawInfo(const InfoState &state, const InfoState &previous, bool full) {
        char value[16];
        char previousValue[16];

        if (full) {
            lcd.fillScreen(TFT_BLACK);
        }

        drawHeader(full);

        if (full) {
            lcd.drawFastHLine(SAFE_MARGIN, 52, lcd.width() - 2 * SAFE_MARGIN, COLOR_SEPARATOR);
        }

        if (full || strcmp(state.version, previous.version) != 0) {
            drawLabelValue("VER", state.version, full ? nullptr : previous.version, 56, TFT_WHITE);
        }

        if (full || state.session != previous.session) {
            formatDuration(state.session, value, sizeof(value));
            formatDuration(previous.session, previousValue, sizeof(previousValue));
            drawLabelValue("UPTIME", value, full ? nullptr : previousValue, 100, TFT_WHITE);
        }

        if (full || state.total != previous.total) {
            formatDuration(state.total, value, sizeof(value));
            formatDuration(previous.total, previousValue, sizeof(previousValue));
            drawLabelValue("TOTAL", value, full ? nullptr : previousValue, 144, TFT_WHITE);
        }

        if (full || state.boots != previous.boots) {
            snprintf(value, sizeof(value), "%lu", static_cast<unsigned long>(state.boots));
            snprintf(previousValue, sizeof(previousValue), "%lu", static_cast<unsigned long>(previous.boots));
            drawLabelValue("BOOTS", value, full ? nullptr : previousValue, 188, TFT_WHITE);
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
    displayMutex = xSemaphoreCreateMutex();

    lcd.init();
    lcd.setRotation(ROTATION);
    lcd.setBrightness(backlight);
    lcd.fillScreen(TFT_BLACK);
    currentScreen = DISPLAY_SCREEN_HOME;
    drawRequested = true;
}

void DisplayLoop(void) {
    if (boxActive) {
        return;
    }

    // A screenshot holds the panel for the whole capture; skip this frame.
    DisplayLock lock(0);
    if (!lock.held) {
        return;
    }

    uint32_t now = millis();

    if (currentScreen == DISPLAY_SCREEN_HOME) {
        HomeState state = readHomeState();
        if (!drawRequested && !stateDirty && state == lastState) {
            return;
        }

        drawHome(state, lastState, drawRequested);
        lastState = state;
    } else if (currentScreen == DISPLAY_SCREEN_NETWORK) {
        NetworkState state = readNetworkState();
        if (!drawRequested && !stateDirty && state == lastNetworkState) {
            return;
        }

        drawNetwork(state, lastNetworkState, drawRequested);
        lastNetworkState = state;
    } else {
        InfoState state = readInfoState();
        if (!drawRequested && !stateDirty && state == lastInfoState) {
            return;
        }

        drawInfo(state, lastInfoState, drawRequested);
        lastInfoState = state;
    }

    drawRequested = false;
    stateDirty = false;
    lastDrawMs = now;
}

void DisplayRequestDraw(void) {
    // Only re-evaluate the fields. A full repaint would flash the panel.
    stateDirty = true;
}

void DisplayScreenNext(void) {
    currentScreen = static_cast<DisplayScreen>((static_cast<uint8_t>(currentScreen) + 1) % DisplayScreenCount());
    drawRequested = true;
}

void DisplayScreenPrev(void) {
    currentScreen = static_cast<DisplayScreen>((static_cast<uint8_t>(currentScreen) + DisplayScreenCount() - 1) % DisplayScreenCount());
    drawRequested = true;
}

void DisplayScreenSet(DisplayScreen screen) {
    if (screen >= DisplayScreenCount()) {
        return;
    }

    currentScreen = screen;
    drawRequested = true;
}

DisplayScreen DisplayScreenGet(void) {
    return currentScreen;
}

const char *DisplayScreenName(DisplayScreen screen) {
    switch (screen) {
        case DISPLAY_SCREEN_HOME:
            return "HOME";
        case DISPLAY_SCREEN_NETWORK:
            return "NETWORK";
        case DISPLAY_SCREEN_INFO:
            return "INFO";
        default:
            return "UNKNOWN";
    }
}

size_t DisplayScreenCount(void) {
    return 3;
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

bool DisplayScreenshotBegin(uint32_t timeoutMs) {
    return lockDisplay(timeoutMs);
}

void DisplayScreenshotRead(uint16_t y, uint16_t h, uint16_t *dst) {
    lcd.readRect(0, y, lcd.width(), h, dst);
}

void DisplayScreenshotEnd(void) {
    unlockDisplay();
}
