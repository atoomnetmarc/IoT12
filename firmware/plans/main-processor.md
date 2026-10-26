# Main processor (ESP32) firmware modernization plan

Milestone plan to bring the Main processor firmware up to date with Control Board PCB V2.2.
Every fact traces to `hardware/Control Board/README.md` (pinout tables A1, J1, J2) or to a decision recorded inline below.

## Facts (from documentation)

- A1 ESP32 pinout: TFT_CS = IO15, TFT_DC = IO2, TFT_RST = IO4, TFT_SCLK = IO18, TFT_MISO = IO19, TFT_MOSI = IO23, TFT_LED = IO32, TOUCH_CS = IO5, TOUCH_IRQ = IO34, I2S_BCK = IO25, I2S_WS = IO26, I2S_DATA = IO27, I2S_GAIN0 = IO13, I2S_GAIN1 = IO14, I2S_SD = IO33, ESP32_SDA = IO21, ESP32_SCL = IO22, HEAT_INT = IO35 (In), ESP32_RESET = EN.
- J1 display connector: 2.8" ILI9341 320x240 touch TFT (changelog V2.2).
- Changelog V2.2: 5110 LCD removed, rotary encoder removed, relay for 24V added, SPI flash added.
- J2 speaker: spoken soldering instructions for the visually impaired (MAX98357 I2S amplifier with gain pins).
- HEAT_INT signaling: the SAMD21 raises the line and latches a 32-bit flag bitmask; the ESP32 reads REGISTER_INTERRUPT to fetch and clear it.
- Platform: espressif32 ~8.0.0 (Arduino core 3.x); WiFi event migration to ARDUINO_EVENT_* is already done in `wific.cpp`.

## Decisions (from interview)

- Restructure the firmware: `src/` split by category (`app/`, `hardware/`, `services/`), a composition root that owns and wires all modules.
- Display and touch run on LovyanGFX; the UI is touch-driven.
- The rotary encoder code (IoAbstraction switches/encoder, PIN_ROT_*) is removed; IO16/IO17/IO33 have no function on V2.2.
- Settings on the Heat controller are JSON; the ESP32 reads and writes them through the I2C minion protocol.
- The ESP32 drives the I2S gain pins (IO13/IO14) and SD (IO33) directly; the REGISTER_I2S_* minion registers are obsolete.
- The first UI iteration is a single home screen with setpoint +/- touch zones, tip temperature, and heater state indicator.
- The I2S registers are removed in Heat controller M2; the ESP32 stops using them in M5/M6. The remaining minion protocol change (link supervision, INT_LINK_TIMEOUT) lands in the same integration session as milestone M7 of the Heat controller plan, so the two processors never disagree about the register map.
- Link supervision: the ESP32 reads REGISTER_HEATER_STATE at least once per 5 seconds. The SAMD21 forces the heater off when no I2C transaction arrives within 5 seconds and raises the `INT_LINK_TIMEOUT` flag; heating resumes automatically when communication resumes. The two MCUs cannot reset each other; the SAMD21 owns heater safety.

## Test strategy

- Every milestone is flashed and functionally tested on the V2.2 hardware; `pio run` passing is the entry ticket, not the exit criterion.
- The Heat controller firmware on the minion bus is the test partner. The Heat controller firmware that exists at the start of this plan speaks the same minion protocol, so it is a valid test partner for every milestone up to and including M5.
- M6 changes the register map together with the Heat controller firmware; from M6 onward the updated Heat controller firmware is the test partner.
- Each milestone below carries a flash marker: `flash: yes` means the build is safe and useful to test on V2.2 hardware, `flash: no` means compile-only.

## Milestones

A checked box means the milestone is committed; the `flash:` marker states whether it was verified on hardware.

### [ ] M1 - Restructure without behavior change (flash: no)

- Move existing modules into the categorized layout: `src/app/` (composition root, `main.cpp`), `src/hardware/` (display, I2S), `src/services/` (HeatController, wific, webserverc, ota).
- Introduce a composition root that owns all modules and wires dependencies.
- `pio run` passes; no functional change.
- Compile-only: the display and encoder code still target the V1 hardware, so this build is not flashed onto V2.2 hardware.

### [ ] M2 - Remove dead hardware code (flash: no)

- Delete the rotary encoder and switch code (PIN_ROT_A/B/SW, IoAbstraction encoder setup in `main.cpp`).
- Delete the 5110 LCD pin defines (PIN_LCD_* uses the old Nokia pinout).
- Keep HEAT_INT (IO35) as an input define.
- `pio run` passes.
- Compile-only: the display is not yet on the V2.2 pins, so there is nothing useful to verify on hardware.

### [ ] M3 - Display on LovyanGFX (flash: yes)

- Add LovyanGFX; configure the ILI9341 over V2.2 VSPI (TFT_SCLK IO18, MOSI IO23, MISO IO19, CS IO15, DC IO2, RST IO4) and the touch controller (TOUCH_CS IO5, TOUCH_IRQ IO34).
- Drive TFT_LED (IO32) for backlight.
- Replace the U8g2 84x48 drawing code with a 320x240 LovyanGFX panel; keep the existing data source (HeatController temperatures and state).
- Remove the custom `u8x8_byte_arduino_hw_spi_custom` yield hack together with U8g2.
- First flash onto V2.2 hardware: verify the display shows the home panel and the backlight switches on.
- `pio run` passes.

### [ ] M4 - Touch UI (flash: yes)

- Build a single home screen: setpoint adjustment with +/- touch zones, tip temperature, and heater on/off state indicator.
- Flash and verify the touch zones respond and the setpoint changes are visible.
- `pio run` passes.

### [ ] M5 - I2S speaker (flash: yes)

- Wire the MAX98357A: BCK IO25, WS IO26, DATA IO27, SD IO33, gain pins IO13/IO14.
- Drive gain and mute directly from the ESP32; implement spoken feedback or at minimum amplifier mute control.
- Flash and verify the amplifier mutes and unmutes on command.
- `pio run` passes.

### [ ] M6 - Heat controller link (flash: yes, joint with Heat controller M7)

- I2C master on IO21/IO22 talking the minion protocol from the Heat controller plan.
- Handle HEAT_INT (IO35) per the resolved semantics.
- Guarantee the link supervision contract: read REGISTER_HEATER_STATE at least once per 5 seconds from a fixed point in the application loop, independent of the display or touch code, so the SAMD21 watchdog never trips during normal operation.
- Handle the `INT_LINK_TIMEOUT` flag: report the lost link in the UI and re-write HEATER_STATE after communication resumes.
- Surface HeatController data (tip temperature, setpoint, state) from the minion protocol instead of the current direct calls.
- Stop using the REGISTER_I2S_* registers; the ESP32 drives the amplifier pins directly (done in M5).
- This milestone lands together with milestone M7 of the Heat controller plan, in one integration session on the bench: both processors get the new register map in the same test run, so the register map never disagrees between the two firmwares.
- `pio run` passes.

### [ ] M7 - Network services cleanup (flash: yes)

- Keep WiFi, OTA, and webserver on core 3.x (event migration already done).
- Move settings read/write for the Heat controller through the minion protocol into the webserver where applicable.
- Flash and verify a settings change made over the webserver reaches the Heat controller and persists.
- `pio run` passes.
