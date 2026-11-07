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

#include "HandleDiagnosis.h"
#include "HeatController.h"
#include "HomeWiFiService.h"
#include "LifetimeStats.h"
#include "TemperatureUnit.h"
#include "TouchCalibration.h"
#include "TouchTest.h"
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

    /*
     * Nominal raw corners of the touch panel, measured on V2.2 hardware. The
     * XPT2046 x reading depends slightly on y, so the four corners are kept
     * independent instead of a rectangle. The order is native (0,0), (0,319),
     * (239,0) and (239,319).
     */
    constexpr uint16_t TOUCH_P0_X = 120;
    constexpr uint16_t TOUCH_P0_Y = 3600;
    constexpr uint16_t TOUCH_P1_X = 210;
    constexpr uint16_t TOUCH_P1_Y = 170;
    constexpr uint16_t TOUCH_P2_X = 3860;
    constexpr uint16_t TOUCH_P2_Y = 3645;
    constexpr uint16_t TOUCH_P3_X = 3900;
    constexpr uint16_t TOUCH_P3_Y = 175;

    // Rectangle bounds of the four corners, for the panel config fallback.
    constexpr uint16_t TOUCH_X_MIN = TOUCH_P0_X;
    constexpr uint16_t TOUCH_X_MAX = TOUCH_P3_X;
    constexpr uint16_t TOUCH_Y_MIN = TOUCH_P2_Y;
    constexpr uint16_t TOUCH_Y_MAX = TOUCH_P1_Y;

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
                cfg.x_min = TOUCH_X_MIN;
                cfg.x_max = TOUCH_X_MAX;
                // The touch panel's raw y axis runs opposite to the display x
                // axis, so swap the range to mirror the mapped x back.
                cfg.y_min = TOUCH_Y_MIN;
                cfg.y_max = TOUCH_Y_MAX;
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

        // Expose the configuration of the bus, panel, touch and backlight.
        const lgfx::Bus_SPI::config_t &busConfig(void) const { return _bus.config(); }
        const lgfx::Panel_Device::config_t &panelConfig(void) const { return _panel.config(); }
        lgfx::ITouch::config_t touchConfig(void) const { return _touch.config(); }
        const lgfx::Light_PWM::config_t &lightConfig(void) const { return _light.config(); }

        // Read a controller ID with the dummy command that the panel needs
        // before it drives MISO.
        uint32_t readId(uint8_t cmd) {
            _bus.beginTransaction();
            digitalWrite(PIN_TFT_CS, HIGH);
            _bus.writeCommand(0, 8);
            _bus.wait();
            digitalWrite(PIN_TFT_CS, LOW);
            _bus.writeCommand(cmd, 8);
            _bus.beginRead(8);

            uint8_t buffer[4] = {};
            _bus.readBytes(buffer, sizeof(buffer), false);

            _bus.endTransaction();
            digitalWrite(PIN_TFT_CS, HIGH);

            return static_cast<uint32_t>(buffer[0]) | (static_cast<uint32_t>(buffer[1]) << 8) | (static_cast<uint32_t>(buffer[2]) << 16) | (static_cast<uint32_t>(buffer[3]) << 24);
        }

        uint32_t readId1(void) { return readId(0x04); }
        uint32_t readId4(void) { return readId(0xD3); }

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
        uint8_t shakePhase = 0;
        HandleDiagnosis diagnosis{HandleState::OK, nullptr, nullptr};
        bool supplyOffMessage = false;
        UiZone pressedZone = UI_ZONE_NONE;
    };

    // Compare every displayed value, so DisplayLoop can skip an unchanged frame.
    bool operator==(const HomeState &a, const HomeState &b) {
        return a.tipTenths == b.tipTenths && a.setTenths == b.setTenths && a.dutyPercent == b.dutyPercent && a.heaterOn == b.heaterOn && a.shakePhase == b.shakePhase && a.diagnosis.state == b.diagnosis.state && a.supplyOffMessage == b.supplyOffMessage && a.pressedZone == b.pressedZone;
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

    // The judgement owns the tip area and the status rail. The clear rectangle
    // spans the full panel width and both font cells, so no red pixel survives
    // a return to the normal layout.
    constexpr FieldRect JUDGEMENT_RECT = {0, 48, 320, 116};

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
    // Latches the 24V-off message after a POWER press until the state changes.
    bool supplyOffMessage = false;
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

        state.pressedZone = UiPressedZone();
        state.diagnosis = HandleDiagnosisClassify();

        if (state.diagnosis.state != HandleState::SUPPLY_OFF) {
            supplyOffMessage = false;
        } else if (state.pressedZone == UI_ZONE_POWER) {
            supplyOffMessage = true;
        }
        state.supplyOffMessage = supplyOffMessage;

        state.tipTenths = static_cast<int16_t>(lroundf(temperature_unit::active()->fromKelvin(HeatController.getTemperatureTip()) * 10.0f));
        state.setTenths = static_cast<int16_t>(lroundf(temperature_unit::active()->fromKelvin(HeatController.getTargetTemperature()) * 10.0f));
        state.dutyPercent = static_cast<int16_t>(lroundf(HeatController.getHeaterDuty()));
        state.heaterOn = HeatController.getHeaterState() == HEATER_STATE::ON;

        bool shakeRaw;
        bool shakeDebounced;
        bool shakeExpired;
        HeatController.getShakeState(&state.shakePhase, &shakeRaw, &shakeDebounced, &shakeExpired);
        (void)shakeRaw;
        (void)shakeDebounced;
        (void)shakeExpired;

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
    void drawValue(const char *value, const char *previous, int16_t y, uint16_t color, int16_t height) {
        lcd.setFont(&fontLogisoso24);
        lcd.setTextSize(1);
        lcd.setTextDatum(lgfx::textdatum_t::middle_right);
        lcd.setTextColor(color, TFT_BLACK);

        int16_t right = lcd.width() - SAFE_MARGIN;
        int16_t left = right - lcd.textWidth(value);

        if (previous != nullptr) {
            int16_t previousLeft = right - lcd.textWidth(previous);

            if (previousLeft < left) {
                lcd.fillRect(previousLeft, y, left - previousLeft, height, TFT_BLACK);
            }
        }

        lcd.drawString(value, right, y + 12);
    }

    // Draw a label on the left and a right-aligned value on the same row. The
    // value goes first, so its clear never eats into the label.
    void drawLabelValue(const char *label, const char *value, const char *previous, int16_t y, uint16_t valueColor, int16_t height) {
        drawValue(value, previous, y, valueColor, height);

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

    // Draw the POWER button with the fill and text color the caller picks, so
    // the button can show the heater state, the 24V fault or the disabled state.
    void drawPowerButtonAt(const UiRect &rect, bool pressed, uint16_t fill, uint16_t text) {
        uint16_t actualFill = pressed ? TFT_WHITE : fill;
        uint16_t actualText = pressed ? TFT_BLACK : text;

        drawButtonFrame(rect, actualFill, pressed);

        lcd.setFont(&fontLogisoso24);
        lcd.setTextSize(1);
        lcd.setTextDatum(lgfx::textdatum_t::middle_center);
        lcd.setTextColor(actualText, actualFill);
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

    // Draw the big tip value at a fixed anchor.
    void drawTipField(const HomeState &state, int16_t x, int16_t y, lgfx::textdatum_t datum) {
        char buffer[16];

        snprintf(buffer, sizeof(buffer), "%5.1f %s", state.tipTenths / 10.0f, temperature_unit::active()->symbol);

        lcd.setFont(&fontLogisoso46);
        lcd.setTextSize(1);
        lcd.setTextDatum(datum);
        lcd.setTextColor(TFT_WHITE, TFT_BLACK);
        lcd.drawString(buffer, x, y);
    }

    void drawSetField(const HomeState &state, int16_t x, int16_t y, lgfx::textdatum_t datum) {
        char buffer[16];

        snprintf(buffer, sizeof(buffer), "%5.1f", state.setTenths / 10.0f);

        lcd.setFont(&fontLogisoso24);
        lcd.setTextSize(1);
        lcd.setTextDatum(datum);
        lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        lcd.drawString(buffer, x, y);
    }

    void drawHeaterField(const HomeState &state, int16_t x, int16_t y, lgfx::textdatum_t datum) {
        char buffer[16];
        // Four characters wide, so OFF and the duty share one cell width and
        // the right-aligned value never reaches the PWM label.
        const char *text = " OFF";
        uint16_t color = TFT_DARKGREY;

        if (state.heaterOn) {
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

    // True when the handle state replaces the tip value and the status rail.
    bool showsJudgement(const HomeState &state) {
        if (state.diagnosis.state == HandleState::SUPPLY_OFF) {
            return state.supplyOffMessage;
        }
        return state.diagnosis.state == HandleState::LOSS || state.diagnosis.state == HandleState::NO_HANDLE || state.diagnosis.state == HandleState::NO_TIP;
    }

    // Draw the situation in the big font and the action in the small font.
    void drawJudgement(const HandleDiagnosis &diagnosis) {
        lcd.setFont(&fontLogisoso46);
        lcd.setTextSize(1);
        lcd.setTextDatum(lgfx::textdatum_t::top_center);
        lcd.setTextColor(TFT_RED, TFT_BLACK);
        lcd.drawString(diagnosis.line1, 160, 56);

        // Green marks an action the user can take; LOSS is a statement.
        uint16_t actionColor = diagnosis.state == HandleState::LOSS ? TFT_RED : TFT_GREEN;
        lcd.setFont(&fontLogisoso24);
        lcd.setTextDatum(lgfx::textdatum_t::top_center);
        lcd.setTextColor(actionColor, TFT_BLACK);
        lcd.drawString(diagnosis.line2, 160, 120);
    }

    // Draw the SET and PWM labels on the status rail.
    void drawRailLabels(void) {
        lcd.setFont(&fontLogisoso24);
        lcd.setTextSize(1);
        lcd.setTextDatum(lgfx::textdatum_t::top_left);
        lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
        lcd.drawString("SET", SAFE_MARGIN, 120);
        lcd.drawString("PWM", 160, 120);
    }

    // Redraw only the buttons whose appearance changed, for any layout.
    void drawHomeButtons(const HomeState &state, const HomeState &previous, bool full, const UiRect &power, const UiRect &plus, const UiRect &minus) {
        bool powerPressed = state.pressedZone == UI_ZONE_POWER;
        bool plusPressed = state.pressedZone == UI_ZONE_PLUS;
        bool minusPressed = state.pressedZone == UI_ZONE_MINUS;
        bool powerPressedBefore = previous.pressedZone == UI_ZONE_POWER;
        bool plusPressedBefore = previous.pressedZone == UI_ZONE_PLUS;
        bool minusPressedBefore = previous.pressedZone == UI_ZONE_MINUS;

        uint16_t powerFill = COLOR_BUTTON;
        uint16_t powerText = TFT_WHITE;

        if (state.diagnosis.state == HandleState::SUPPLY_OFF) {
            powerFill = TFT_RED;
            powerText = TFT_BLACK;
        } else if (showsJudgement(state)) {
            powerText = TFT_DARKGREY;
        } else if (state.heaterOn && (state.shakePhase == static_cast<uint8_t>(SHAKE_PHASE::RAMP) || state.shakePhase == static_cast<uint8_t>(SHAKE_PHASE::STANDBY2))) {
            powerFill = TFT_YELLOW;
            powerText = TFT_BLACK;
        } else if (state.heaterOn) {
            powerFill = TFT_GREEN;
            powerText = TFT_BLACK;
        }

        if (full || state.heaterOn != previous.heaterOn || state.shakePhase != previous.shakePhase || powerPressed != powerPressedBefore || state.diagnosis.state != previous.diagnosis.state) {
            drawPowerButtonAt(power, powerPressed, powerFill, powerText);
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

        bool judgement = showsJudgement(state);
        bool judgementBefore = showsJudgement(previous);
        bool layoutChanged = judgement != judgementBefore;
        bool messageChanged = state.diagnosis.state != previous.diagnosis.state;

        /*
         * The judgement owns the tip area and the status rail. Clear the full
         * panel width and both font cells when the layout or the message
         * changes, so no red pixel survives a return to the normal layout.
         */
        if (full || layoutChanged || (judgement && messageChanged)) {
            clearField(JUDGEMENT_RECT);
            lcd.drawFastHLine(SAFE_MARGIN, 116, lcd.width() - 2 * SAFE_MARGIN, COLOR_SEPARATOR);

            if (judgement) {
                drawJudgement(state.diagnosis);
            } else {
                drawRailLabels();
                drawTipField(state, 160, 56, lgfx::textdatum_t::top_center);
                drawSetField(state, 150, 120, lgfx::textdatum_t::top_right);
                drawHeaterField(state, 304, 120, lgfx::textdatum_t::top_right);
            }
        } else if (!judgement) {
            /*
             * The text is drawn with a background color instead of clearing the
             * field first. LovyanGFX paints the glyph cells, so a fixed-width
             * string overwrites the previous one without a black flash.
             */
            if (state.tipTenths != previous.tipTenths) {
                drawTipField(state, 160, 56, lgfx::textdatum_t::top_center);
            }

            if (state.setTenths != previous.setTenths) {
                // Right-aligned, so the value grows to the left and never touches
                // the SET label when it crosses from two to three digits.
                drawSetField(state, 150, 120, lgfx::textdatum_t::top_right);
            }

            if (state.heaterOn != previous.heaterOn || state.dutyPercent != previous.dutyPercent) {
                drawHeaterField(state, 304, 120, lgfx::textdatum_t::top_right);
            }
        }

        /*
         * Redraw each button only when its own appearance changed. Repainting
         * the whole strip on every press made the other two buttons blink.
         */
        drawHomeButtons(state, previous, full, UI_POWER_RECT, UI_PLUS_RECT, UI_MINUS_RECT);
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
            drawLabelValue("SSID", state.ssid, full ? nullptr : previous.ssid, 56, TFT_WHITE, VALUE_H);
        }

        if (full || strcmp(state.pass, previous.pass) != 0) {
            drawLabelValue("PASS", state.pass, full ? nullptr : previous.pass, 92, TFT_WHITE, VALUE_H);
        }

        if (full || strcmp(state.linkLabel, previous.linkLabel) != 0 || strcmp(state.linkValue, previous.linkValue) != 0) {
            // The label switches between AP and STA, so clear the label column
            // first; a shorter label would otherwise leave a glyph behind.
            if (strcmp(state.linkLabel, previous.linkLabel) != 0) {
                lcd.fillRect(SAFE_MARGIN, 128, LABEL_COLUMN_W, VALUE_H, TFT_BLACK);
            }
            drawLabelValue(state.linkLabel, state.linkValue, full ? nullptr : previous.linkValue, 128, TFT_WHITE, VALUE_H);
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

    // Format a duration as "27h 14m", or "14m 03s" below one hour. forceHours
    // keeps the hour form for a total that is below one hour.
    void formatDuration(uint32_t seconds, char *buffer, size_t size, bool forceHours) {
        uint32_t hours = seconds / 3600U;
        uint32_t minutes = (seconds / 60U) % 60U;

        if (forceHours || hours > 0) {
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
            drawLabelValue("VER", state.version, full ? nullptr : previous.version, 56, TFT_WHITE, VALUE_H);
        }

        if (full || state.session != previous.session) {
            formatDuration(state.session, value, sizeof(value), false);
            formatDuration(previous.session, previousValue, sizeof(previousValue), false);
            drawLabelValue("UPTIME", value, full ? nullptr : previousValue, 100, TFT_WHITE, VALUE_H);
        }

        if (full || state.total != previous.total) {
            formatDuration(state.total, value, sizeof(value), true);
            formatDuration(previous.total, previousValue, sizeof(previousValue), true);
            drawLabelValue("TOTAL", value, full ? nullptr : previousValue, 144, TFT_WHITE, VALUE_H);
        }

        if (full || state.boots != previous.boots) {
            snprintf(value, sizeof(value), "%lu", static_cast<unsigned long>(state.boots));
            snprintf(previousValue, sizeof(previousValue), "%lu", static_cast<unsigned long>(previous.boots));
            drawLabelValue("BOOTS", value, full ? nullptr : previousValue, 188, TFT_WHITE, VALUE_H);
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

    // Values the touch screen shows.
    struct TouchState {
        bool stored = false;
        uint16_t values[8] = {};
        UiZone pressedZone = UI_ZONE_NONE;
    };

    // Compare every displayed value, so DisplayLoop can skip an unchanged frame.
    bool operator==(const TouchState &a, const TouchState &b) {
        if (a.stored != b.stored || a.pressedZone != b.pressedZone) {
            return false;
        }

        for (size_t i = 0; i < 8; ++i) {
            if (a.values[i] != b.values[i]) {
                return false;
            }
        }

        return true;
    }

    TouchState lastTouchState;

    // Read the stored calibration values.
    TouchState readTouchState(void) {
        TouchState state;

        state.stored = touch_calibration::stored();
        for (size_t i = 0; i < 8; ++i) {
            state.values[i] = touch_calibration::valueAt(i);
        }
        state.pressedZone = UiPressedZone();

        return state;
    }

    // Draw a labelled action button. CAL and TEST share the AP and + geometry.
    void drawActionButton(const UiRect &rect, const char *label, bool pressed) {
        uint16_t fill = pressed ? TFT_WHITE : COLOR_BUTTON;
        uint16_t text = pressed ? TFT_BLACK : TFT_WHITE;

        drawButtonFrame(rect, fill, pressed);

        lcd.setFont(&fontLogisoso24);
        lcd.setTextSize(1);
        lcd.setTextDatum(lgfx::textdatum_t::middle_center);
        lcd.setTextColor(text, fill);
        lcd.drawString(label, rect.x + rect.w / 2, rect.y + rect.h / 2);
    }

    // Draw the touch screen: four corner pairs, the CAL button and the TEST button.
    void drawTouch(const TouchState &state, const TouchState &previous, bool full) {
        if (full) {
            lcd.fillScreen(TFT_BLACK);
        }

        drawHeader(full);

        if (full) {
            lcd.drawFastHLine(SAFE_MARGIN, 48, lcd.width() - 2 * SAFE_MARGIN, COLOR_SEPARATOR);
        }

        for (uint8_t i = 0; i < 4; ++i) {
            char label[4];
            char value[16];
            char previousValue[16];

            snprintf(label, sizeof(label), "P%u", i);
            snprintf(value, sizeof(value), "%u,%u", state.values[i * 2], state.values[i * 2 + 1]);
            snprintf(previousValue, sizeof(previousValue), "%u,%u", previous.values[i * 2], previous.values[i * 2 + 1]);

            if (full || state.values[i * 2] != previous.values[i * 2] || state.values[i * 2 + 1] != previous.values[i * 2 + 1]) {
                drawLabelValue(label, value, full ? nullptr : previousValue, 52 + i * 28, TFT_WHITE, 28);
            }
        }

        if (full || state.pressedZone != previous.pressedZone) {
            drawActionButton(UI_CAL_RECT, "CAL", state.pressedZone == UI_ZONE_CAL);
            drawActionButton(UI_TEST_RECT, "TEST", state.pressedZone == UI_ZONE_TEST);
        }
    }

    // Draw the touch test screen: title, last point, crosshair and EXIT button.
    // The text and the button are drawn after the crosshair, so they stay
    // readable when the finger is on top of them.
    void drawTouchTest(bool havePoint, uint16_t x, uint16_t y, uint16_t rawX, uint16_t rawY, bool exitPressed) {
        lcd.fillScreen(TFT_BLACK);

        if (havePoint) {
            lcd.drawCircle(x, y, 12, TFT_YELLOW);
            lcd.drawFastHLine(x - 16, y, 32, TFT_YELLOW);
            lcd.drawFastVLine(x, y - 16, 32, TFT_YELLOW);
        }

        lcd.setFont(&fontLogisoso24);
        lcd.setTextSize(1);
        lcd.setTextDatum(lgfx::textdatum_t::middle_center);
        lcd.setTextColor(TFT_WHITE, TFT_BLACK);
        lcd.drawString("TEST", 160, 32);

        char line1[24];
        char line2[24];

        if (havePoint) {
            snprintf(line1, sizeof(line1), "X %u Y %u", x, y);
            snprintf(line2, sizeof(line2), "raw %u %u", rawX, rawY);
        } else {
            snprintf(line1, sizeof(line1), "touch the panel");
            snprintf(line2, sizeof(line2), "EXIT to stop");
        }

        lcd.drawString(line1, 160, 64);
        lcd.drawString(line2, 160, 92);

        drawActionButton(UI_EXIT_RECT, "EXIT", exitPressed);
    }

    // Two-line explanation for a calibration step, using the home screen font.
    void drawCalibrationText(const char *line1, const char *line2) {
        lcd.setFont(&fontLogisoso24);
        lcd.setTextSize(1);
        lcd.setTextDatum(lgfx::textdatum_t::middle_center);
        lcd.setTextColor(TFT_WHITE, TFT_BLACK);
        lcd.drawString(line1, 160, 100);
        lcd.drawString(line2, 160, 136);
    }

    const char *CALIBRATION_WHY[4][2] = {
        {"Touch origin", "press for 1 second"},
        {"X scale", "press for 1 second"},
        {"Y scale", "press for 1 second"},
        {"Check the map", "press for 1 second"},
    };

    const char *VALIDATION_WHY[4][2] = {
        {"Tap top left", "to prove the map"},
        {"Tap top right", "to prove the map"},
        {"Tap bottom left", "to prove the map"},
        {"Tap bottom right", "to prove the map"},
    };
}

void DisplayTouchCalibrationApply(const uint16_t values[8]) {
    uint16_t copy[8];

    memcpy(copy, values, sizeof(copy));
    lcd.setTouchCalibrate(copy);
}

void DisplayTouchCalibrationFallback(uint16_t values[8]) {
    values[0] = TOUCH_P0_X;
    values[1] = TOUCH_P0_Y;
    values[2] = TOUCH_P1_X;
    values[3] = TOUCH_P1_Y;
    values[4] = TOUCH_P2_X;
    values[5] = TOUCH_P2_Y;
    values[6] = TOUCH_P3_X;
    values[7] = TOUCH_P3_Y;
}

void DisplayCalibrationBegin(void) {
    DisplayLock lock(100);
    if (!lock.held) {
        return;
    }

    lcd.fillScreen(TFT_BLACK);
}

void DisplayCalibrationTarget(uint8_t index, bool pressed, bool ready) {
    if (index >= 4) {
        return;
    }

    DisplayLock lock(100);
    if (!lock.held) {
        return;
    }

    lcd.fillScreen(TFT_BLACK);

    const touch_calibration::Rect &r = touch_calibration::TARGET_RECTS[index];
    int16_t cx = r.x + r.w / 2;
    int16_t cy = r.y + r.h / 2;
    uint16_t color = ready ? TFT_GREEN : (pressed ? TFT_YELLOW : TFT_WHITE);

    lcd.drawCircle(cx, cy, 12, color);
    lcd.drawFastHLine(cx - 16, cy, 32, color);
    lcd.drawFastVLine(cx, cy - 16, 32, color);
    drawCalibrationText(CALIBRATION_WHY[index][0], CALIBRATION_WHY[index][1]);
}

void DisplayCalibrationValidation(uint8_t index, bool pressed, bool hit) {
    if (index >= 4) {
        return;
    }

    DisplayLock lock(100);
    if (!lock.held) {
        return;
    }

    lcd.fillScreen(TFT_BLACK);

    const touch_calibration::Rect &r = touch_calibration::VALIDATION_RECTS[index];
    UiRect rect = {r.x, r.y, r.w, r.h};
    uint16_t fill = hit ? TFT_GREEN : (pressed ? TFT_WHITE : COLOR_BUTTON);
    uint16_t text = (hit || pressed) ? TFT_BLACK : TFT_WHITE;

    drawButtonFrame(rect, fill, pressed);

    lcd.setFont(&fontLogisoso24);
    lcd.setTextSize(1);
    lcd.setTextDatum(lgfx::textdatum_t::middle_center);
    lcd.setTextColor(text, fill);
    lcd.drawString("OK", rect.x + rect.w / 2, rect.y + rect.h / 2);
    drawCalibrationText(VALIDATION_WHY[index][0], VALIDATION_WHY[index][1]);
}

void DisplayCalibrationMessage(const char *line1, const char *line2) {
    DisplayLock lock(100);
    if (!lock.held) {
        return;
    }

    lcd.fillScreen(TFT_BLACK);
    drawCalibrationText(line1, line2);
}

void DisplayCalibrationEnd(void) {
    drawRequested = true;
}

void DisplayTouchTestDraw(bool havePoint, uint16_t x, uint16_t y, uint16_t rawX, uint16_t rawY, bool exitPressed) {
    DisplayLock lock(100);
    if (!lock.held) {
        return;
    }

    drawTouchTest(havePoint, x, y, rawX, rawY, exitPressed);
}

void DisplayTouchTestEnd(void) {
    drawRequested = true;
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

    // The calibration mode owns the panel.
    if (touch_calibration::active()) {
        return;
    }

    // The touch test mode owns the panel.
    if (touch_test::active()) {
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
    } else if (currentScreen == DISPLAY_SCREEN_TOUCH) {
        TouchState state = readTouchState();
        if (!drawRequested && !stateDirty && state == lastTouchState) {
            return;
        }

        drawTouch(state, lastTouchState, drawRequested);
        lastTouchState = state;
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
        case DISPLAY_SCREEN_TOUCH:
            return "TOUCH";
        default:
            return "UNKNOWN";
    }
}

size_t DisplayScreenCount(void) {
    return 4;
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

// Fill the runtime properties of the panel, its bus, the touch controller and
// the backlight. The controller IDs are read over SPI.
void DisplayGetInfo(DisplayInfo *info) {
    if (info == nullptr) {
        return;
    }

    *info = {};
    info->driver = DisplayDriverName();

    DisplayLock lock(100);
    if (!lock.held) {
        return;
    }

    const lgfx::Bus_SPI::config_t &bus = lcd.busConfig();
    const lgfx::Panel_Device::config_t &panel = lcd.panelConfig();
    const lgfx::ITouch::config_t touch = lcd.touchConfig();
    const lgfx::Light_PWM::config_t &light = lcd.lightConfig();

    info->width = lcd.width();
    info->height = lcd.height();
    info->rotation = lcd.getRotation();
    info->backlight = backlight;
    info->lastDrawMs = lastDrawMs;
    info->colorDepth = static_cast<uint8_t>(lcd.getColorDepth());

    info->pinCs = panel.pin_cs;
    info->pinRst = panel.pin_rst;
    info->pinBusy = panel.pin_busy;
    info->memoryWidth = panel.memory_width;
    info->memoryHeight = panel.memory_height;
    info->panelWidth = panel.panel_width;
    info->panelHeight = panel.panel_height;
    info->offsetX = panel.offset_x;
    info->offsetY = panel.offset_y;
    info->offsetRotation = panel.offset_rotation;
    info->dummyReadPixel = panel.dummy_read_pixel;
    info->dummyReadBits = panel.dummy_read_bits;
    info->endReadDelayUs = panel.end_read_delay_us;
    info->readable = panel.readable;
    info->invert = panel.invert;
    info->rgbOrder = panel.rgb_order;
    info->dlen16bit = panel.dlen_16bit;
    info->busShared = panel.bus_shared;

    info->freqWrite = bus.freq_write;
    info->freqRead = bus.freq_read;
    info->pinSclk = bus.pin_sclk;
    info->pinMiso = bus.pin_miso;
    info->pinMosi = bus.pin_mosi;
    info->pinDc = bus.pin_dc;
    info->pinIo0 = bus.pin_io0;
    info->pinIo1 = bus.pin_io1;
    info->pinIo2 = bus.pin_io2;
    info->pinIo3 = bus.pin_io3;
    info->spiMode = bus.spi_mode;
    info->spi3wire = bus.spi_3wire;
    info->useLock = bus.use_lock;
    info->dmaChannel = bus.dma_channel;
    info->spiHost = static_cast<int>(bus.spi_host);

    info->touchFreq = touch.freq;
    info->touchXMin = touch.x_min;
    info->touchXMax = touch.x_max;
    info->touchYMin = touch.y_min;
    info->touchYMax = touch.y_max;
    info->touchBusShared = touch.bus_shared;
    info->touchPinInt = touch.pin_int;
    info->touchPinRst = touch.pin_rst;
    info->touchOffsetRotation = touch.offset_rotation;
    info->touchSpiHost = static_cast<int>(touch.spi_host);
    info->touchPinSclk = touch.pin_sclk;
    info->touchPinMosi = touch.pin_mosi;
    info->touchPinMiso = touch.pin_miso;
    info->touchPinCs = touch.pin_cs;

    info->lightFreq = light.freq;
    info->lightPinBl = light.pin_bl;
    info->lightOffset = light.offset;
    info->lightPwmChannel = light.pwm_channel;
    info->lightInvert = light.invert;

    info->id1 = lcd.readId1();
    info->id4 = lcd.readId4();
    info->id9 = lcd.readId(0x09);
    info->idc = lcd.readId(0x0C);
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
