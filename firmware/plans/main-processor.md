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
- The home panel redraws only when a displayed value changes; there is no periodic redraw. The tip and setpoint values are formatted to a fixed width of six characters and drawn with an explicit background color, so a value update repaints only its own glyph cells. The heater and link fields are cleared only when the link state changes, because that is when the text format changes.
- The rotary encoder code (IoAbstraction switches/encoder, PIN_ROT_*) is removed; IO16/IO17/IO33 have no function on V2.2.
- Settings on the Heat controller are JSON; the ESP32 reads and writes them through the I2C minion protocol.
- The ESP32 drives the I2S gain pins (IO13/IO14) and SD (IO33) directly; the REGISTER_I2S_* minion registers are obsolete.
- The first UI iteration is a single home screen with setpoint +/- touch zones, tip temperature, a heater state indicator, and a POWER zone.
- Touch UI: `src/app/Ui.cpp` owns the zones, the hit-test and the actions; `display.cpp` stays a pure draw and hardware module. `UiLoop()` runs before `DisplayLoop()`; the UI writes the setpoint and calls `DisplayRequestDraw()`, and the display's read-and-diff draws the result.
- Touch input: the IRQ line gates the read. `TouchRead()` runs only while the IRQ is low, with a 20 ms sample interval and a 30 ms debounce.
- Setpoint interaction: a tap changes the setpoint by 5 C; a hold repeats after 500 ms and then every 150 ms. Every step writes `REGISTER_TEMPERATURE_HEATER_TARGET` immediately, so the panel never shows a value the heater does not have.
- Setpoint limits: 0-450 C (273.15-723.15 K). The UI clamps the touch zones and the heat controller clamps every write to the target register as the hard limit.
- Heater control: the home screen has a POWER zone that writes `REGISTER_HEATER_STATE` through `setHeaterState`, next to the heater state indicator.
- `ui press <x> <y>` injects a synthetic tap through the same hit-test as a real touch, one-shot, and prints the zone and the action.
- The I2S registers are removed in Heat controller M2; the ESP32 stops using them in M5/M6. The remaining minion protocol change (link supervision, INT_LINK_TIMEOUT) lands in the same integration session as milestone M7 of the Heat controller plan, so the two processors never disagree about the register map.
- Link supervision: the ESP32 reads REGISTER_HEATER_STATE at least once per 5 seconds. The SAMD21 forces the heater off when no I2C transaction arrives within 5 seconds and raises the `INT_LINK_TIMEOUT` flag; heating resumes automatically when communication resumes. The two MCUs cannot reset each other; the SAMD21 owns heater safety.
- A Serial CLI is a development requirement, not a product feature: it is always compiled in, never prints periodically, and only responds to commands plus one-shot events. It is the `SerialCli` module, shared with the Heat controller project.
- M5 audio: native ESP-IDF I2S driver, no external audio library. The MAX98357A gain table comes from the `main` branch `I2SGPIO.cpp`: 6 dB = GAIN0 high/GAIN1 input, 9 dB = both input, 12 dB = GAIN0 low/GAIN1 input, 15 dB = GAIN0 input/GAIN1 low. 3 dB is not reachable because the amplifier runs on 5 V. SD enable = input, shutdown = output low. The default gain is 9 dB. The agent tests only 6 dB; the user verifies 9/12/15 dB.
- M5 playback: a main-loop state machine at 16 kHz mono 16-bit with 256-sample chunks and a zero-timeout write gate. The DMA buffer is doubled to 12 descriptors of 240 frames and prefilled at tone start, so a slow main-loop pass does not drain it. No FreeRTOS task. Spoken feedback is deferred to a later milestone.
- Code style is clang-format with the project `.clang-format` (LLVM base, 4 spaces, `BreakBeforeBraces: Custom` with only `BeforeLambdaBody: false`, which means Attach/K&R, `InsertBraces: true`, no column limit). Every `.cpp` and `.h` in `Heat controller/`, `Main processor/` and `lib/` is formatted with it; `lib/.clang-format` is a copy so the shared libraries use the same style.
- Network provisioning: no automatic AP. The AP starts from the touchscreen with SSID `iot12-<last 3 MAC bytes>`. WiFi credentials are a JSON network array in NVS. There is no compile-time network configuration and no `local_env.ini`.
- AP and STA are mutually exclusive. The AP stops only on an explicit action from the touchscreen or from the page, never on a timer or on a successful STA connect.
- The web stack is ESPAsyncWebServer with AsyncTCP. A handler that needs the minion bus posts a request to the main loop; it never touches I2C from the async task.
- Web assets are split into `webui/index.html`, `webui/style.css` and `webui/app.js`, each gzipped into its own PROGMEM array by `tools/generate_webui_header.py`.
- The page is read-only without a credential and shows every live value: tip temperature, setpoint, NTC temperature, supply voltage, iron current, heater state and duty, link state, WiFi state, IP and uptime. Writes require a credential and are out of scope for M7.
- Live data uses Server-Sent Events through `AsyncEventSource`. The client receives the full state in one message on connect, and every push after that repeats the full state once per second from the heartbeat read. There are no deltas and no separate snapshot event: the payload is small, the client stays stateless, and a dropped message self-heals on the next second. The minion bus sees no extra traffic.
- Discovery is mDNS `iot12-<last 3 MAC bytes>.local`; the touchscreen shows the IP as the fallback. The responder starts only after the station connects and stops before the AP mode switch, because every WiFi mode change destroys and recreates the lwIP netif and a responder that outlives it crashes on the next event.
- One adaptive page serves the AP portal and the STA dashboard; the page detects portal mode from the `portal` field in the state stream.
- The AP uses WPA2 with a one-time random password of 8 lowercase letters and 4 digits from `esp_random()`, stored in NVS and shown on the touchscreen.
- OTA is a web upload allowed only while the AP portal is open. Upload over STA follows when a credential exists.
- The touchscreen is a screen carousel: a 32 px header bar with a left arrow box at (16,16), a right arrow box at (272,16) and the screen title centered between them. The readout moves down to make room. The screens are Home, Network and Info, in that order, and the arrows wrap around the list.
- NVS layout: namespace `iot12`, key `wifi_cfg` holds the JSON network array, key `ap_pass` holds the 12-character AP password. The AP SSID is `iot12-<last 3 MAC bytes>` computed at boot and never stored.

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
- CLI: extend `display` to LovyanGFX (driver, resolution, rotation, backlight, last draw time), `display backlight <0-255>`, `display on|off`, `display box <margin>|off` (bezel measurement); add `touch` (raw coordinates, pressed state).
- `pio run` passes.
- Verified on V2.2 hardware: `version` reports `FLASH_MB: 16`, `display` reports 320x240 rotation 1, the home panel is visible, `display box 16` draws the bezel inset, `display backlight 64` dims the panel, and `touch` reports `IRQ: high` when idle and raw/mapped coordinates when pressed (raw about 1900/1500 maps to about 110/130).

### [x] M4 - Touch UI (flash: yes)

- Add `src/app/Ui.cpp` with `include/Ui.h`: the zone table, the hit-test, the press/release/repeat state machine, and the actions. `display.cpp` stays a pure draw and hardware module; `display.h` gains `DisplayRequestDraw()`. The Variant C button rects live in `Ui.h`, so the display and the hit-test use the same numbers.
- Build a single home screen: tip temperature, setpoint, heater state indicator, a POWER zone, and the setpoint +/- zones.
- A tap changes the setpoint by 5 C. A hold repeats after 500 ms and then every 150 ms. Every step writes `REGISTER_TEMPERATURE_HEATER_TARGET` immediately.
- Clamp the setpoint to 0-450 C (273.15-723.15 K) in the UI. The heat controller clamps the same range as the hard limit (Heat controller M9).
- The POWER zone writes `REGISTER_HEATER_STATE` through `setHeaterState`; the indicator shows the PWM duty from `REGISTER_HEATER_DUTY 0x42` (green `37%`) while the heater is on, and `OFF` when it is off.
- Layout and fonts: Variant C, three equal 96x84 buttons in the bottom strip and a readout on top, all inside the 16 px bezel safe area. The face is Logisoso (U8g2), 46 for the tip value and 24 for everything else, because it carries a real `°` (U+00B0) that no GFX font does. LovyanGFX reads the `u8g2_font_*` arrays directly through `lgfx::U8g2font`, so the U8g2 `lib_deps` stays.
- Read the touch panel only while the IRQ line is low, with a 20 ms sample interval and a 30 ms debounce.
- `UiLoop()` runs before `DisplayLoop()` in the application loop. The UI writes the setpoint and calls `DisplayRequestDraw()`; the display's read-and-diff draws the result. `DisplayRequestDraw()` sets `stateDirty`, not `drawRequested`, so a touch only redraws the changed fields and never wipes the whole panel.
- Flash and verify the touch zones respond, the setpoint changes are visible, the POWER zone switches the heater, and a hold repeats.
- CLI: `ui` (current screen, setpoint, heater state, last touch event), `ui setpoint <celsius>`, `ui press <x> <y>` (a synthetic tap through the same hit-test, one-shot, prints the zone and the action), `ui zones` (list touch zones and bounds).
- `pio run` passes.
- Verified on V2.2 hardware: the home panel shows `TIP`, `SET`, the tip value with a real `°`, the heater duty and the three buttons; `ui zones` lists `POWER 16,140 96x84`, `PLUS 112,140 96x84`, `MINUS 208,140 96x84`; `ui press` on each zone changes the setpoint by 5 C and toggles the heater; `ui setpoint 500` clamps to 450.0 and `ui setpoint -10` to 0.0. `heat regs` lists `0x42 HEATER_DUTY`; with the heater on it reads 80-95%, with the heater off 0%. The tip value and setpoint use `%5.1f` so the decimal point stays put, and the fields are drawn with a background color so a value update does not flash. The touch config swaps `y_min`/`y_max` because the XPT2046 is mounted 180 degrees to the panel. A touch press or release only sets `stateDirty`, so the button strip redraws without wiping the readout, and each button is diffed on its own pressed state so pressing one does not repaint the other two. A setpoint set from the UI is persisted by the heat controller and survives a heat-controller reboot. Flash 1,364,311 bytes (20.8%).

### [x] M5 - I2S speaker (flash: yes)

- Configure the MAX98357A pins: BCK IO25, WS IO26, DATA IO27, SD IO33, GAIN0 IO13, GAIN1 IO14. The module is on the PCB; the firmware only configures the pins.
- Drive gain and mute directly from the ESP32 with the gain table from the `main` branch `I2SGPIO.cpp`: 6 dB = GAIN0 high/GAIN1 input, 9 dB = both input, 12 dB = GAIN0 low/GAIN1 input, 15 dB = GAIN0 input/GAIN1 low. 3 dB is not reachable because the amplifier runs on 5 V. SD enable = input, shutdown = output low. The default gain is 9 dB.
- Play a test tone with a main-loop state machine: 16 kHz mono 16-bit, 256-sample chunks, a zero-timeout write gate. The DMA buffer is doubled to 12 descriptors of 240 frames and prefilled at tone start. No FreeRTOS task. Spoken feedback is deferred to a later milestone.
- Safety: the agent tests only 6 dB; the user verifies 9/12/15 dB.
- Flash and verify the amplifier mutes and unmutes on command, and the test tone plays at 6 dB.
- CLI: `audio` (enabled, gain in dB, gain pin states, SD state, I2S state, sample rate, playing, bytes written, stalls, last error), `audio gain <6|9|12|15>`, `audio mute|unmute`, `audio test` (1 kHz, 200 ms, 10% amplitude), `audio tone <freq 20-7000> <ms 10-5000> [<amplitude 1-100>]`, `audio pins` (raw pin modes and levels). The tone upper limit is 7000 Hz because the 16 kHz sample rate has an 8 kHz Nyquist limit; 8000 Hz is silent and above it aliases.
- `pio run` passes.
- Verified on V2.2 hardware: `audio` reports I2S ready, gain 9 dB and SD output low; `audio unmute` enables the amplifier; `audio gain 6` gives GAIN0 output high and GAIN1 input; `audio test` plays a 1 kHz tone; `audio mute` silences it; `audio pins` matches the gain table. The user verified 9, 12 and 15 dB. The doubled DMA buffer and the prefill remove the initial stutter. Flash 1,423,632 bytes (21.2%).

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

### [x] M7 - Network services (flash: yes)

- Replace `wific`, `webserverc`, `ota` and `config` with the network services: `HomeWiFiService` (NVS network array, STA state machine), `WebServerService` (AsyncWebServer, portal, SSE), `WebConfigStore` (NVS JSON), `MdnsService` (responder lifecycle tied to the station connection), and `DeviceName` (`iot12-<last 3 MAC bytes>`).
- Remove `local_env.ini` and the `WIFI_SSID`, `WIFI_PASSWORD`, `HOSTNAME` and `OTA_PASSWORD` build flags. The build has no compile-time network configuration.
- Add `esp32async/AsyncTCP` and `esp32async/ESPAsyncWebServer` to `lib_deps`.
- Add `webui/` and `tools/generate_webui_header.py`; register the generator in `extra_scripts`.
- Touchscreen: a screen carousel with a 32 px header bar. The left arrow box sits at (16,16) and the right arrow box at (272,16), both 32x32, with the screen title centered between them. The home readout is a hero value centered at y=56, a hairline at y=116, and a status rail at y=120 with `SET` plus the setpoint on the left and `PWM` plus the heater status on the right. The three buttons sit at y=164..224 (60 px tall). The screens are Home, Network (AP start/stop, SSID, WPA2 password, AP IP, STA state) and Info (firmware version, uptime, IP, MAC, link state, I2C error counters). The arrows wrap around the list.
- NVS: namespace `iot12`, key `wifi_cfg` (JSON `{"networks":[{"ssid":"...","pass":"..."}]}`) and key `ap_pass` (the 12-character AP password). The AP SSID is `iot12-<last 3 MAC bytes>` computed at boot and never stored.
- SSE route `/api/stream`: the full state in one message on connect, then the full state again once per second from the heartbeat read. No deltas and no separate snapshot event.
- The page owns the stream lifecycle: it closes the `EventSource` on error and retries itself with a 1 s to 15 s backoff, resetting the delay on every successful open. The browser's own retry is not used because it keeps the old URL and has no backoff, so a device that moved from the AP address to its station address would never be picked up. A retry also fires immediately when the page becomes visible again or the browser reports it is back online. The header shows the stream state (`live`, `connecting`, `retry in N s`).
- The Live card is a 3x2 grid of tiles, each with the label above its value, so a label can never drift away from its number. Below 640 px the grid drops to two columns.
- Read-only routes: `/api/status` (WiFi state, IP, RSSI, portal flag, uptime, heap), `/api/device` (version, git, build, board, chip), `/api/config` (SSIDs only, never passwords).
- Screenshot route `/api/screenshot`: a chunked `application/octet-stream` of raw RGB565, 16 rows per strip, with `X-Screenshot-Width`, `X-Screenshot-Height` and `X-Screenshot-Format` headers. The page converts it to a canvas and a PNG, so the device never allocates a full framebuffer. The display mutex is held for the whole capture, so a draw cannot tear the image; the response owns a guard that releases the mutex also when the client disconnects early.
- The home layout is the hero variant chosen on the device: the temperature is the single large value, the setpoint and heater status share a hairline-separated rail, and the buttons are 60 px tall. The heater status is four characters wide (`OFF`, `LOST` or the duty), so it never reaches the `PWM` label.
- Write routes gated by the open portal: `/api/wifi` (save the network array to NVS) and `/api/ota` (firmware upload).
- Keep register `0x42 HEATER_DUTY` in the state message so the page shows the real output.
- Keep WiFi, OTA and the webserver on core 3.x; the event migration is already done.
- CLI: extend `wifi`/`web`/`ota` with state, reconnect count, request count, last request, and partition info; add `net` (combined wifi/web/ota summary), `ap on|off` (start and stop the portal from the CLI), and `screen` (current screen, title and index), `screen next|prev|home` (drive the carousel from the CLI).
- Settings read and write over the minion bus stay out of M7; they follow with the credential work.
- Flash and verify: the touchscreen starts the AP, a laptop joins with the shown password, the portal saves a network, the device joins it, `iot12-<last 3 MAC bytes>.local` resolves, the page shows live values, and a firmware upload over the portal succeeds.
- `pio run` passes.
- Verified on V2.2 hardware: `version` reports 2026.11.1 on the 16 MB flash, `net` reports `WIFI: idle`, `PORTAL: off` and `OTA: idle`, `screen` reports `HOME` index 0 of 3, `screen next|prev|home` cycles the carousel, `ui zones` lists `PREV 16,16 32x32`, `NEXT 272,16 32x32`, `POWER 16,164 96x60`, `PLUS 112,164 96x60`, `MINUS 208,164 96x60` and `AP 16,196 288x28`, `ui press` on the arrow zones cycles the carousel, and `ui press` on the AP zone starts and stops the portal. `ap` reports the AP state, the SSID `iot12-<last 3 MAC bytes>`, the stored password and the AP IP, and `web` reports the same SSID and IP 192.168.4.1 while the portal is open. The portal save stores the network, the station connects, `iot12-<last 3 MAC bytes>.local` resolves, the page serves, and `/api/stream` pushes the live state once per second. A full portal cycle (`ap on`, `ap off`) stops and restarts the responder without a crash. The served `app.js` carries the retry logic and the header carries the stream status element, and two sequential stream connections both deliver the state frame with a stable heap. The served page carries the Display card and `/api/screenshot` returns 153,600 bytes of RGB565 with the expected headers. The home screen uses the hero layout and the capture shows the centered value, the hairline rail and the 60 px buttons. The laptop join and the OTA upload are the remaining bench checks.

### [x] M8 - Network and Info screen layout (flash: yes)

- Keep three carousel screens: Home, Network and Info. The layout variants are not added.
- Network: the SSID, the WPA2 password and the link row take three rows at y=56, y=92 and y=128. The link row shows where the device is reachable: the label `AP` with the AP address while the portal is open, the label `STA` with the station address once the station is connected, and the label `STA` with the state name (`idle`, `scanning`, `connecting`) while it is not. Every value is white.
- The AP button is 96x60 at (112,164), the same size and place as the middle Home button, and carries the text `AP`. The button is green while the portal is open and is the only coloured element on the screen. The link label ends at y=152, so the gap to the button is 12 px.
- Info: four rows at y=56, y=100, y=144 and y=188 for the firmware version, the uptime, the IP address and the link state. The MAC address and the I2C error count are removed from the screen.
- Both screens use Logisoso24 for the labels and the values. The labels are dark grey and the values are right-aligned at x=304. No other font is added.
- A value is right-aligned, and only the cells that a wider previous value left behind are cleared, so a value that changes every second does not blink.
- Both screens redraw only the rows whose value changed, against a cached state, in the same way as the Home screen. The one-second full repaint is removed.
- The AP zone moves from (16,196) 288x28 to (112,164) 96x60, so `ui zones` reports the new rectangle.
- `screen network` and `screen info` select the two screens directly, next to `screen next|prev|home`.
- Verify with `/api/screenshot` on both screens and `pio run`.
- Verified on V2.2 hardware: `screen` reports `HOME` index 0 of 3, `screen network` and `screen info` select the two screens, and `ui zones` reports `AP 112,164 96x60`. The Network capture shows `SSID iot12-f2ed3c` at 174 px, the password at 188 px and `STA 172.27.80.159` in white at y=56/92/128, with the 96x60 `AP` button below and 6 px of clear space between the rows. The Info capture shows `VER 2026.11.1`, `UPTIME`, `IP 172.27.80.159` and a green `LINK OK` at y=56/100/144/188.

### [x] M8b - Lifetime uptime and boot counter (flash: yes)

- The Info screen replaces the IP and link rows with the lifetime uptime and the boot count. The four rows are the firmware version, the session uptime, the lifetime uptime and the boot count at y=56, y=100, y=144 and y=188.
- The IP row goes because the Network screen shows the station address. The link row goes because the Home screen shows the heat controller link as `--.-` and `LOST`.
- The counters live in the NVS namespace `iot12` under the keys `boots` and `secs`. The heat controller flash is not used, so the minion register map does not change.
- At boot the device reads both counters, adds one to the boot count and writes both back. A boot is rare, so this write costs nothing.
- The session uptime comes from `esp_timer_get_time()`, which does not wrap at 49.7 days like `millis()`.
- The device writes the lifetime uptime every 15 minutes. A power cut loses at most 15 minutes. The write is one NVS commit of 4 bytes, and the NVS wear leveling absorbs it.
- The lifetime uptime is the stored value plus the session uptime, so the row ticks every second and is always correct.
- The uptime format is `27h 14m` above one hour and `14m 03s` below it. The lifetime format is `1234h 56m`. The boot count is a plain integer.
- The value clear uses the width of the previous value, so a ticking row never repaints its label. A fixed left limit reaches into the `UPTIME` and `TOTAL` labels and makes their last glyphs blink.
- `stats` reports `BOOTS`, `TOTAL` and `SESSION`, `stats flush` writes the total now, and `stats reset` clears both counters.
- Verify with `pio run`, `stats`, a reboot that increments `BOOTS`, and `/api/screenshot` on the Info screen.
- Verified on V2.2 hardware: `pio run` passes at 23.0% flash and 17.3% RAM. `stats` reports `BOOTS: 4`, `TOTAL: 0` and `SESSION: 0` on the first boot after the flash. `stats flush` writes the total, and `reboot yes-really` moves `BOOTS` from 5 to 6 while `TOTAL` continues from the stored value plus the new session. The Info capture shows `VER 2026.11.1`, `UPTIME 0m 05s`, `TOTAL 0m 06s` and `BOOTS 9` at y=56/100/144/188. Two captures 1.3 s apart differ in the value cells and are pixel-identical in the label cells, so the labels do not blink. The 15-minute interval is code-verified; the flush path is hardware-verified.

### [ ] M9 - Touchscreen calibration (flash: yes)

- Add a calibration screen that asks for four corner taps, computes the XPT2046 raw-to-panel mapping, and stores it in NVS as the new default.
- The current fixed `x_min`/`x_max`/`y_min`/`y_max` in the panel config become the fallback when no calibration is stored.
- Add `touch calibrate` to the CLI and a `CALIBRATE` entry on the Info screen, so the small buttons can be re-aligned without a reflash.
- Verify that a tap in the centre of each button reports the matching zone with `ui press` and on the device.
