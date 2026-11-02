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
- Flash: the ESP32 module has 16 MiB. The build uses `board_upload.flash_size = 16MB` and `board_build.partitions = default_16MB.csv`: app slots of 6.25 MiB each, SPIFFS 3.375 MiB, coredump 64 KiB. The firmware uses 1.17 MiB of the app slot (17.9%).

## Decisions (from interview)

- Restructure the firmware: `src/` split by category (`app/`, `hardware/`, `services/`), a composition root that owns and wires all modules.
- Display and touch run on LovyanGFX; the UI is touch-driven.
- Panel: ILI9341 2.8" 320x240 (SKU MSP2807) with XPT2046 resistive touch on the same SPI bus, separate CS. Rotation 1 (landscape) is confirmed on hardware. Backlight is IO32 through LEDC PWM at 5 kHz, 8-bit. Touch calibration constants live in the code; `touch` prints raw and mapped coordinates so they can be tuned on the bench.
- The display SPI bus is full-duplex (`spi_3wire = false`): the XPT2046 shares MISO with the panel, and a half-duplex bus returns no touch data. The touch configuration repeats the SCLK/MOSI/MISO pins so the bus is never re-initialized with default pins.
- The home panel redraws only when a displayed value changes; there is no periodic redraw. The tip and setpoint values are formatted to a fixed width of six characters and drawn with an explicit background color, so a value update repaints only its own glyph cells. The heater and link fields have a fixed rectangle that is cleared before drawing. `display test` holds the test pattern for 3 seconds.
- The rotary encoder code (IoAbstraction switches/encoder, PIN_ROT_*) is removed; IO16/IO17/IO33 have no function on V2.2.
- Settings on the Heat controller are JSON; the ESP32 reads and writes them through the I2C minion protocol.
- The ESP32 drives the I2S gain pins (IO13/IO14) and SD (IO33) directly; the REGISTER_I2S_* minion registers are obsolete.
- The first UI iteration is a single home screen with setpoint +/- touch zones, tip temperature, and heater state indicator.
- The I2S registers are removed in Heat controller M2; the ESP32 stops using them in M5/M6. The remaining minion protocol change (link supervision, INT_LINK_TIMEOUT) lands in the same integration session as milestone M7 of the Heat controller plan, so the two processors never disagree about the register map.
- Link supervision: the ESP32 reads REGISTER_HEATER_STATE at least once per 5 seconds. The SAMD21 forces the heater off when no I2C transaction arrives within 5 seconds and raises the `INT_LINK_TIMEOUT` flag; heating resumes automatically when communication resumes. The two MCUs cannot reset each other; the SAMD21 owns heater safety.
- A Serial CLI is a development requirement, not a product feature: it is always compiled in, never prints periodically, and only responds to commands plus one-shot events. It is the `SerialCli` module, shared with the Heat controller project.
- Code style is clang-format with the project `.clang-format` (LLVM base, 4 spaces, `BreakBeforeBraces: Custom` with only `BeforeLambdaBody: false`, which means Attach/K&R, `InsertBraces: true`, no column limit). Every `.cpp` and `.h` in `Heat controller/`, `Main processor/` and `lib/` is formatted with it; `lib/.clang-format` is a copy so the shared libraries use the same style.

## Test strategy

- Every milestone is flashed and functionally tested on the V2.2 hardware; `pio run` passing is the entry ticket, not the exit criterion.
- The Heat controller firmware on the minion bus is the test partner. The Heat controller firmware that exists at the start of this plan speaks the same minion protocol, so it is a valid test partner for every milestone up to and including M5.
- M6 changes the register map together with the Heat controller firmware; from M6 onward the updated Heat controller firmware is the test partner.
- Each milestone below carries a flash marker: `flash: yes` means the build is safe and useful to test on V2.2 hardware, `flash: no` means compile-only.
- Every milestone's CLI commands are exercised on hardware as part of the milestone verification; a command that cannot be demonstrated is not done.

## Serial CLI

Development requires a command line on UART0; validating the minion link, the display, the touch zones, and the network services without one is slow and error-prone. The CLI is the `SerialCli` module, vendored unchanged in behavior.

- Module: `lib/SerialCli/` shared with the Heat controller through `lib_extra_dirs = ../lib`; Arduino-free (push-driven `feed(char)`, output callback, runtime command table, auto-generated `help`).
- Transport: `Serial` (UART0) at 115200. `SerialCli::loop()` drains `Serial.available()` from the main `loop()`; the output callback writes to `Serial`.
- Registration: every module owns a `registerXxxCommands(cli::SerialCli&, deps...)` free function; the composition root (M1) calls the registrars after wiring. Commands are registered after the module is initialized.
- Non-blocking rule: handlers return immediately. No `delay()`, no `while (!Serial)`, no unbounded loops. Destructive commands use the stateless two-step form (`command yes-really`), which is not listed in `help`.
- No periodic prints. The firmware never prints telemetry on a timer. All output is a response to a command, plus one-shot events recorded in a small RAM ring buffer (`EventLog`, 32 records) that `events` dumps on demand. Live event printing is off by default (`events live on` enables it) and is a single line per event, never periodic.
- Logging: the shared `lib/Log/` module provides leveled, tagged logging in the ESP-IDF output shape (`I (12345) Tag: message`), with the level adjustable at runtime through `log <level>`. The ESP-IDF `ESP_LOGx` macros are deliberately not used: this platform ships precompiled libraries built with `CONFIG_LOG_MAXIMUM_LEVEL=1`, so `ESP_LOGI/W/D` compile to nothing and only `ESP_LOGE` survives. `CORE_DEBUG_LEVEL=0` keeps the Arduino core itself quiet on UART0.
- The periodic debug prints that exist today are removed in M2c: the per-HEAT_INT bitmask in `HeatController.cpp`, the OTA progress line, and the boot/button/WiFi banners. Their data becomes queryable through the CLI.
- Safety: the CLI never bypasses the link supervision contract. `heat on|off` and `heat setpoint` go through the same `HeatControllerClass` path as the UI, and the heartbeat read of REGISTER_HEATER_STATE stays in its fixed point in the application loop, independent of CLI activity.
- Output format: `KEY: value\r\n` lines, uppercase keys, one fact per line. Errors are `<command>: not available (<reason>)` or `Usage: ...`.

## Milestones

A checked box means the milestone is committed; the `flash:` marker states whether it was verified on hardware.

### [x] M1 - Restructure without behavior change (flash: no)

- Move existing modules into the categorized layout: `src/app/` (`main.cpp`, `App.cpp`), `src/hardware/` (display), `src/services/` (HeatController, wific, webserverc, ota, config).
- Introduce a composition root: `App.cpp` owns the CLI instance and the wiring, and exposes `AppSetup()`/`AppLoop()`; `main.cpp` only calls those two. The modules keep their existing free-function interfaces, so the restructure stays move-only.
- `pio run` passes; no functional change.
- Compile-only: the display and encoder code still target the V1 hardware, so this build is not flashed onto V2.2 hardware.

### [x] M2 - Remove dead hardware code (flash: no)

- Delete the rotary encoder and switch code (PIN_ROT_A/B/SW, IoAbstraction encoder setup, `onEncoderChange`, `onSpinwheelClicked`).
- Delete the 5110 LCD pin defines (PIN_LCD_* uses the old Nokia pinout) and the U8g2 driver with its custom SPI yield hack. The display becomes a no-op module until M3; the 200 ms refresh becomes a `millis()` check in the composition root.
- Drop the U8g2, IoAbstraction and TaskManagerIO dependencies.
- Keep HEAT_INT (IO35) as an input define.
- `pio run` passes.
- Compile-only: the display is not yet on the V2.2 pins, so there is nothing useful to verify on hardware.

### [x] M2b - Serial CLI foundation (flash: yes)

- Vendor `SerialCli` into `lib/SerialCli/`; add `lib_extra_dirs = ../lib` to `platformio.ini`. Keep `MAX_COMMANDS` at 32 and `INPUT_BUFFER_SIZE` at 256, through `-DCLI_MAX_COMMANDS` / `-DCLI_INPUT_BUFFER_SIZE` build flags so the library source stays shared and unmodified.
- Add the shared `EventLog` library (`lib/EventLog/`): a 32-record ring buffer of one-shot events (boot, reset reason, fault, link timeout, heater state change, settings save, WiFi/OTA events). Recording an event never prints.
- Add the CLI wiring in the composition root: the `Serial` output callback, the `loop()` drain, and the `help`/`version`/`reboot`/`uptime`/`mem`/`events` commands.
- `version` prints PRODUCT_VERSION, build date, chip model, flash size, CPU frequency, and `esp_reset_reason()`. `reboot` is two-step (`reboot yes-really`). `mem` prints free heap, minimum free heap, and largest free block. `events` dumps the ring buffer; `events clear` clears it; `events live on|off` toggles one-shot live printing (default off).
- Flash and verify: `help` lists the commands, `version` matches the build, `events` shows the boot record, and the main loop keeps running while a command is typed.
- `pio run` passes.

### [x] M2c - Remove periodic prints (flash: yes)

- Delete the 32-bit interrupt bitmask print in `HeatController.cpp` `loop()` that runs on every HEAT_INT (every measurement cycle, about 50 Hz); the flags are kept in `lastInterrupt` and are available through `heat` and `heat int`.
- Delete the "Button pressed" print; the encoder is gone in M2.
- Replace the boot banner ("Booting", CPU frequency, flash size) with `version` and a single `EventLog` boot record.
- Delete the WiFi password print in `wific.cpp`; record connect/disconnect as events. `wifi` shows SSID, IP, RSSI, and the connect counters.
- Do not log every association attempt: the ESP32 retries a failed connect every ~2.4 s and raises a DISCONNECTED event each time, which is a periodic print in disguise. Only a real connected -> disconnected transition is logged and recorded; failed attempts are `LOG_D` and only when the status value changes.
- Replace the OTA progress print in `ota.cpp` with `EventLog` records at start, end, and error only; no per-chunk progress. `ota` shows state, progress, and last error.
- Delete the "HTTP server started" print; `web` shows running state, request count, and last URI.
- Add the shared `lib/Log/` logger and use it for the one-shot messages: boot, WiFi connect/disconnect, HTTP server start, OTA start/end/error, heat controller reboot. `CORE_DEBUG_LEVEL=0` keeps the Arduino core quiet.
- Add the commands that expose the same data on demand: `heat` (version, compatible, heater state, tip temperature, setpoint, NTC, serial number, const1/const2, last interrupt), `heat int` (fetch-and-clear), `wifi`, `web`, `ota`, `display`, `log` (get/set the log level).
- Add `heat on|off` and `heat setpoint <celsius>` here rather than in M6: the rotary encoder was the only way to control the heater and it is removed in M2, so the CLI has to take over before the touch UI exists.
- Flash and verify: UART0 stays silent while the system runs, each command returns the data the old prints showed, and the WiFi password never appears.
- `pio run` passes.

### [x] M3 - Display on LovyanGFX (flash: yes)

- Add LovyanGFX; configure the ILI9341 over V2.2 VSPI (TFT_SCLK IO18, MOSI IO23, MISO IO19, CS IO15, DC IO2, RST IO4) and the touch controller (TOUCH_CS IO5, TOUCH_IRQ IO34).
- Partition table: `default_16MB.csv` with `board_upload.flash_size = 16MB`, so the app slot is 6.25 MiB and LovyanGFX fits without further layout changes. The first flash after this change is a full erase over the serial port; an OTA update across the old 4 MB layout is not safe.
- Drive TFT_LED (IO32) for backlight.
- Replace the U8g2 84x48 drawing code with a 320x240 LovyanGFX panel; keep the existing data source (HeatController temperatures and state).
- Remove the custom `u8x8_byte_arduino_hw_spi_custom` yield hack together with U8g2.
- First flash onto V2.2 hardware: verify the display shows the home panel and the backlight switches on.
- CLI: extend `display` to LovyanGFX (driver, resolution, rotation, backlight, last draw time), `display backlight <0-255>`, `display on|off`, `display test` (draw a test pattern once); add `touch` (raw coordinates, pressed state).
- `pio run` passes.
- Verified on V2.2 hardware: `version` reports `FLASH_MB: 16`, `display` reports 320x240 rotation 1, the home panel is visible, `display test` shows the color bars, `display backlight 64` dims the panel, and `touch` reports `IRQ: high` when idle and raw/mapped coordinates when pressed (raw about 1900/1500 maps to about 110/130).

### [ ] M4 - Touch UI (flash: yes)

- Build a single home screen: setpoint adjustment with +/- touch zones, tip temperature, and heater on/off state indicator.
- Flash and verify the touch zones respond and the setpoint changes are visible.
- CLI: `ui` (current screen, setpoint, heater state, last touch event), `ui setpoint <celsius>`, `ui press <x> <y>` (inject a touch event to validate hit zones without a finger), `ui zones` (list touch zones and bounds).
- `pio run` passes.

### [ ] M5 - I2S speaker (flash: yes)

- Wire the MAX98357A: BCK IO25, WS IO26, DATA IO27, SD IO33, gain pins IO13/IO14.
- Drive gain and mute directly from the ESP32; implement spoken feedback or at minimum amplifier mute control.
- Flash and verify the amplifier mutes and unmutes on command.
- CLI: `audio` (SD/mute state, gain pins, gain in dB, playing), `audio gain <3|6|9|12|15>`, `audio mute|unmute`, `audio test` (play a test tone once).
- `pio run` passes.

### [x] M6 - Heat controller link (flash: yes, joint with Heat controller M7)

- I2C master on IO21/IO22 talking the minion protocol from the Heat controller plan.
- Handle HEAT_INT (IO35) per the resolved semantics.
- Guarantee the link supervision contract: read REGISTER_HEATER_STATE at least once per 5 seconds from a fixed point in the application loop, independent of the display or touch code, so the SAMD21 watchdog never trips during normal operation.
- Handle the `INT_LINK_TIMEOUT` flag: report the lost link in the UI and re-write HEATER_STATE after communication resumes.
- Surface HeatController data (tip temperature, setpoint, state) from the minion protocol instead of the current direct calls.
- Stop using the REGISTER_I2S_* registers; the ESP32 drives the amplifier pins directly (done in M5).
- This milestone lands together with milestone M7 of the Heat controller plan, in one integration session on the bench: both processors get the new register map in the same test run, so the register map never disagrees between the two firmwares.
- CLI: extend `heat` with link age, interrupt flags, NTC temperature, and the full register map (`heat regs`); add `heat read <reg> [len]`, `heat write <reg> <hex...> yes-really` (bench-only), `heat int` (fetch-and-clear), `heat on|off`, `heat setpoint <celsius>`, `link` (last heartbeat age, timeout flag, re-write count, error counters), `link timeout [<ms>]`.
- `pio run` passes.
- The heartbeat is a read of REGISTER_HEATER_STATE every second from `HeatControllerClass::loop()`, which the application loop calls before the display and the network services.
- The handshake retries every second while the heat controller does not answer, so a reset of either processor never leaves the link down.
- Verified on V2.2 hardware: `heat` reports the link age, the timeout flag, the re-write and error counters, and the heartbeat count; `heat regs` and `heat read` match the heat controller register map.
- Verified on V2.2 hardware: with the link timeout at 500 ms, the heat controller raises INT_LINK_TIMEOUT, the Main processor fetches the flag, reports the lost link, and re-writes HEATER_STATE, so heating resumes.
- Verified on V2.2 hardware: `heat on|off` and `heat setpoint` return the write result, and `heat write` changes a register.

### [ ] M7 - Network services cleanup (flash: yes)

- Keep WiFi, OTA, and webserver on core 3.x (event migration already done).
- Move settings read/write for the Heat controller through the minion protocol into the webserver where applicable.
- Flash and verify a settings change made over the webserver reaches the Heat controller and persists.
- CLI: extend `wifi`/`web`/`ota` with state, reconnect count, request count, last request, and partition info; add `settings` (proxy over the minion bus: `settings get <key>`, `settings set <key> <value>`, `settings file`) and `net` (combined wifi/web/ota summary).
- `pio run` passes.
