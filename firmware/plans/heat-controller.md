# Heat controller (SAMD21) firmware modernization plan

Milestone plan to bring the Heat controller firmware up to date with Control Board PCB V2.2.
Every fact traces to `hardware/Control Board/README.md` (pinout tables A3, U2, J4, J5, J6) or to a decision recorded inline below.

## Facts (from documentation)

- A3 SAMD21 pinout: 24V_ON = PA07 (Out), HEAT_PWM = PA18 (Out), HEAT_INT = PA14 (Out), ADC_RDY = PA06 (In), HEAT_SDA/SCL = PA22/PA23, ESP32_SDA/SCL = PA08/PA09, EXP_D5/D7/SCL/SDA = PA15/PA21/PA17/PA16, FLASH_CS = PA20, MISO/SCK/MOSI = PA12/PB11/PB10, IRON_CURRENT = PB02, SHAKE = PA04 parallel with PB09, NTC = PA02, TEMP = PB08, VIN = PA05.
- U2 ADS1115 pinout: I2C address 0x48 on the HEAT I2C bus; AIN0 = TEMP, AIN1 = NTC, AIN2 = IRON_CURRENT, AIN3 = VIN; ALERT/RDY = ADC_RDY.
- J4: 24V switched power, 5V always on, GND, 24V_ON. The power supply relay is driven through 24V_ON and can be cut off by the user interlock switch (Power Supply README, J2).
- J5 pin 1: SHAKE_IN, the T12 handle motion signal, wired to the SAMD21 on PA04 parallel with PB09.
- HEAT_INT signaling: the SAMD21 raises PA14 and latches a 32-bit flag bitmask; the ESP32 reads REGISTER_INTERRUPT to fetch and clear it.
- Changelog V2.2: debug LED removed, 5110 LCD removed, rotary encoder removed, small EEPROM removed, SPI flash added, expansion header added.
- Arduino Zero variant mapping (verified in `framework-arduino-samd/variants/arduino_zero/variant.h` and `variant.cpp`): `Wire` = sercom3 on PA22/PA23, `WIRE_INTERFACES_COUNT 1`; default SPI pads = PA12 (MISO), PB10 (MOSI), PB11 (SCK); digital pins: D2 = PA14, D6 = PA20, D8 = PA06, D9 = PA07, D10 = PA18; analog pins: A0 = PA02, A1 = PB08, A2 = PB09, A3 = PA04, A4 = PA05, A5 = PB02.

## Decisions (from interview)

- Restructure the firmware: `src/` split by category (`app/`, `hardware/`, `services/`). The restructure is move-only: files are relocated and includes are updated, the existing global singletons and their wiring stay as they are. A composition root is deferred until a milestone needs one.
- Measurement source of truth is the ADS1115 only. All four analog channels are read over the HEAT I2C bus. The SAMD21 internal ADC stays available as a read-only diagnostic on the same analog nets; it never feeds the heater control loop. The ADS1115 sample timing fits the 20 ms measurement window of the heating cycle; this is proven by the working measurement code on the current hardware.
- I2C topology: `Wire` (sercom3, PA22/PA23) is the master bus for the ADS1115. A custom `TwoWire` instance on sercom2 (PA08/PA09) is the I2C minion bus to the ESP32. sercom0 is unavailable because the `arduino_mzero` variant defines `SERCOM0_Handler` for `Serial1`. The expansion bus (PA16/PA17) stays untouched.
- Settings are stored as JSON (ArduinoJson) on the external W25Q64 SPI flash.
- Storage stack: Adafruit_SPIFlashBase with Adafruit_FlashTransport_SPI on the default SPI pads (PA12/PB10/PB11) and FLASH_CS on PA20, with the upstream littlefs library (`rw0402/littlefs`) on top through a block-device glue. Adafruit SPIFlash 5.x does not bundle LittleFS and Adafruit_LittleFS is not in the PlatformIO registry; the base class avoids the SdFat wrapper and its 4 KiB block cache. The LittleFS block is the 4 KiB sector, the cache is 256 bytes, and the lookahead buffer is 16 bytes. When the flash is detected as blank (all 0xFF) it is formatted automatically. Partitioning beyond the LittleFS volume is deferred until a feature needs it.
- No EEPROM emulation in internal flash.
- The pin map and the heater PWM output move in one milestone, so no firmware build exists that compiles against the V2.2 pin defines while still driving the V1 pins. That milestone is compile-only because the new pin map and PWM output are not yet verified on V2.2 hardware. The minion bus runs on sercom2 (PA08/PA09) and is unaffected by the pin map move.
- The I2S registers are removed in M2; the ESP32 stops using them in its M5/M6. The remaining minion protocol change (link supervision, INT_LINK_TIMEOUT) lands in the same integration step as the matching ESP32 firmware change, so the two processors never disagree about the register map.
- Link supervision: the SAMD21 forces `heaterState` to OFF when no heartbeat arrives within 5 seconds, and raises the `INT_LINK_TIMEOUT` interrupt flag. The heartbeat is an explicit read of REGISTER_HEATER_STATE by the ESP32 at least once per 5 seconds; only that read restarts the SAMD21 watchdog, other I2C traffic does not. Heating resumes when the ESP32 resumes heartbeating. The two MCUs cannot reset each other; the SAMD21 owns heater safety.
- SHAKE is a digital signal: the handle line is a 100k pullup with a 100 pF RC filter (10 us time constant), read as a plain GPIO input with a small software debounce. The analog comparator is not used on V2.2.
- Settings persistence: double-buffered `/settings.json` and `/settings.json.bak` on LittleFS. Each save copies `/settings.json` to `/settings.json.bak`, then writes the new `/settings.json` in one `lfs_file_write`.
- Settings recovery: `/settings.json` always wins; `/settings.json.bak` is used only when the primary is missing or invalid. LittleFS stores no timestamps, so "newest" is not provable; the copy-then-write order makes the backup the last completed save, and a partial primary is invalid JSON.
- Settings file format: JSON with camelCase keys identical to the `SettingClass` members, so the file, the CLI, and the tests use one name per setting. The target temperature is stored in Kelvin, the same unit as `REGISTER_TEMPERATURE_HEATER_TARGET`. ArduinoJson 7 with an elastic `JsonDocument` serializes the seven fields.
- No EEPROM migration: the first boot after M5 starts from defaults. The EEPROM code, the CRC, and the `EepromAbstractionWire` dependency are removed in M5.
- CLI naming: `settings` prints the file keys verbatim (camelCase). It is the one command that does not use uppercase keys, because a key copied from `settings` must work as `settings set` input unchanged.
- SHAKE switches the heater to a standby temperature on lack of motion; this is the lowest-priority nice-to-have and is scheduled last.
- A Serial CLI is a development requirement, not a product feature: it is always compiled in, never prints periodically, and only responds to commands plus one-shot events. It is the `SerialCli` module, shared with the ESP32 project.
- Code style is clang-format with the project `.clang-format` (LLVM base, 4 spaces, `BreakBeforeBraces: Custom` with only `BeforeLambdaBody: false`, which means Attach/K&R, `InsertBraces: true`, no column limit). Every `.cpp` and `.h` in `Heat controller/`, `Main processor/` and `lib/` is formatted with it; `lib/.clang-format` is a copy so the shared libraries use the same style.

## Test strategy

- Every milestone is flashed and functionally tested on the V2.2 hardware; `pio run` passing is the entry ticket, not the exit criterion.
- The ESP32 firmware on the minion bus is the test partner. The ESP32 firmware that exists at the start of this plan speaks the same minion protocol and the same HEAT_INT semantics, so it is a valid test partner for every milestone up to and including M6.
- M7 changes the register map together with the ESP32 firmware; from M7 onward the updated ESP32 firmware is the test partner.
- Each milestone below carries a flash marker: `flash: yes` means the build is safe and useful to test on V2.2 hardware, `flash: no` means compile-only because the build does not yet drive the V2.2 pins correctly.
- Every milestone's CLI commands are exercised on hardware as part of the milestone verification; a command that cannot be demonstrated is not done.

## Serial CLI

Development requires a command line on the USB-CDC port; validating the minion protocol, the flash stack, and the settings without one is slow and error-prone. The CLI is the `SerialCli` module, vendored unchanged in behavior.

- Module: `lib/SerialCli/` shared by both projects through `lib_extra_dirs = ../lib`; `SerialCli.h`/`SerialCli.cpp` are Arduino-free (push-driven `feed(char)`, output callback, runtime command table, auto-generated `help`).
- Transport: `SerialUSB` at 115200. `SerialCli::loop()` drains `SerialUSB.available()` from the main `loop()`; the output callback writes to `SerialUSB` and returns immediately when no host is attached (`if (!SerialUSB) return;`) so a missing terminal can never stall the heater loop.
- Registration: every module owns a `registerXxxCommands(cli::SerialCli&, deps...)` free function in its own translation unit; `main.cpp` only calls the registrars. Commands are registered after the module is initialized.
- Non-blocking rule: handlers return immediately. No `delay()`, no `while (!Serial)`, no unbounded loops. Destructive commands use the stateless two-step form (`command yes-really`), which is not listed in `help`.
- Output caps: `flash read` is capped at 256 bytes and `fs cat` at 4 KiB per invocation, so a dump cannot stretch the 20 ms measurement window.
- No periodic prints. The firmware never prints telemetry on a timer. All output is a response to a command, plus one-shot events recorded in a small RAM ring buffer (`EventLog`, 32 records) that `events` dumps on demand. Live event printing is off by default (`events live on` enables it) and is a single line per event, never periodic.
- The periodic debug prints that exist today are removed in M3c: the per-cycle block in `Heater.cpp`, the per-cycle dump in `AdcExternal.cpp`, and the EEPROM save line in `Setting.cpp`. Their data becomes queryable through the CLI.
- Safety: commands that change heater state go through `Setting.heaterState` and the same path as the minion protocol; the CLI never writes HEAT_PWM directly. CLI activity does not count as a link heartbeat: only a minion read of REGISTER_HEATER_STATE restarts the supervision watchdog, so a CLI `heater on` while the link is down is overridden by the watchdog.
- Output format: `KEY: value\r\n` lines, uppercase keys, one fact per line. Errors are `<command>: not available (<reason>)` or `Usage: ...`. The `settings` command prints the JSON keys verbatim (camelCase) so a printed key is valid `settings set` input.

## Milestones

A checked box means the milestone is committed; the `flash:` marker states whether it was verified on hardware.

### [x] M1 - Restructure without behavior change (flash: no)

- Move existing modules into the categorized layout: `src/app/` (`main.cpp`), `src/hardware/` (Heater, Sensor, AdcExternal, AdcInternal, AnalogComparator, I2SGPIO, WireMinion), `src/services/` (Setting).
- Move-only: only the translation units move; the headers stay in `include/`, so no include path changes. The global singletons and their wiring stay exactly as they are.
- Delete the empty `src/config.cpp`.
- `pio run` passes; no functional change.
- Compile-only: the heater still drives the V1 PWM pin, so this build is not flashed onto V2.2 hardware.

### [x] M2 - Safe pin map (flash: no)

- Rewrite `include/config.h` from the A3 table: PIN_HEAT_PWM = D10 (PA18), PIN_24V_ON = D9 (PA07), PIN_ADC_READY = D8 (PA06), PIN_HEAT_INT = D2 (PA14), PIN_FLASH_CS = D6 (PA20).
- Move the TCC0 PWM output from PA16 to PA18 (D10) in the same change, so the pin defines and the peripheral mux never disagree.
- Keep the QuickPID control loop; re-check the heating cycle against the relay-based 24V switching (J4) and the user interlock.
- Remove PIN_I2S_* and the I2SGPIO module (I2S lives on the ESP32), remove PIN_INTERNAL_ADC_* and the AdcInternal module (ADS1115-only decision), remove the TP pin setup and LED_BUILTIN from `main.cpp` and `WireMinion.cpp` (debug LED removed in V2.2).
- Drive 24V_ON high in `Heater.init()` and leave it on; it is a slow relay enable, HEAT_PWM does the switching.
- Verify every Arduino pin number against `variant.cpp` before use; the D-numbers above are derived from it.
- `pio run` passes.
- Compile-only: the minion bus runs on sercom2 (PA08/PA09) and is unaffected by the pin map move, but the new pin map and PWM output are not yet verified on V2.2 hardware.

### [x] M3 - I2C topology (flash: yes)

- Keep `Wire` (sercom3, PA22/PA23) as the ADS1115 master bus; no variant changes needed.
- Keep the custom `TwoWire` instance on sercom2 for PA08/PA09 as the I2C minion bus to the ESP32. sercom0 is unavailable: the `arduino_mzero` variant defines `SERCOM0_Handler` for `Serial1`, so a second handler is a duplicate symbol.
- Remove `I2C_ADDRESS_EEPROM` from `config.h`; the address stays local to `Setting.cpp` until the settings move to the SPI flash.
- Flash and verify the minion bus with the existing ESP32 firmware: register reads (version, temperatures) and REGISTER_INTERRUPT fetch-and-clear over HEAT_INT.
- `pio run` passes.
- Verified on V2.2 hardware: the ESP32 gets an ACK on 0x2A, reads version `2026.11.1`, and receives INT_REBOOT (bit 31) on the first fetch and INT_MEASUREMENTS_GROUP_A (bit 30) on every measurement cycle. INT_MEASUREMENTS_GROUP_B (bit 29) only appears while the heater is actually switching, which is expected with the heater off.
- Verified register reads: 0x00 version, 0x01 serial number, 0x11 heater state, 0x40 tip temperature, 0x41 NTC temperature, 0x80 target temperature, 0xF8 const1 (`0x1234`), 0xF9 const2 (`3.141593`).
- Verified the write path: writing 0x80 and reading it back returns the written value, and the original value is restored afterwards.
- The minion bus runs at 100 kHz on both sides: `SERCOM::initSlaveWIRE()` never sets CTRLA.SPEED, so the SAMD21 slave stays in standard mode.
- `PIN_HEAT_INT` is D4 (PA14) on the `mzeroUSB` board; D2 is PA08 (ESP32_SDA) and D3 is PA09 (ESP32_SCL). Using D2 for HEAT_INT silently disables the SDA pinmux.

### [x] M3b - Serial CLI foundation (flash: yes)

- Vendor `SerialCli` into `lib/SerialCli/`; add `lib_extra_dirs = ../lib` to `platformio.ini`. Set `MAX_COMMANDS` to 16 and `INPUT_BUFFER_SIZE` to 128 for the SAMD21 RAM budget, through `-DCLI_MAX_COMMANDS` / `-DCLI_INPUT_BUFFER_SIZE` build flags so the library source stays shared and unmodified.
- Add the shared `EventLog` library (`lib/EventLog/`): a 32-record ring buffer of one-shot events (boot, reset cause, fault, link timeout, heater state change, settings save, flash error). Recording an event never prints.
- Add the CLI wiring in `main.cpp`: the `SerialUSB` output callback, the `loop()` drain, and the `help`/`version`/`reboot`/`uptime`/`mem`/`events` commands.
- `version` prints PRODUCT_VERSION, build date, board, and `PM->RCAUSE`. `reboot` is two-step (`reboot yes-really`). `events` dumps the ring buffer; `events clear` clears it; `events live on|off` toggles one-shot live printing (default off).
- Flash and verify: `help` lists the commands, `version` matches the build, `events` shows the boot record, and the heater loop keeps running while a command is typed.
- `pio run` passes.

### [x] M3c - Remove periodic prints (flash: yes)

- Delete the ~20-line `SerialUSB.println` block in `Heater.cpp` `loop()` that runs on every `eventHeatOff` (every heating cycle, about 50 Hz). The values move to file-scope state and are exposed through `HeaterClass::status()`.
- Delete the per-cycle dump in `AdcExternal.cpp` `loop()` and the `requestDump()` call in `Heater.cpp`; `ADCExternalClass::readChannelRaw()` serves the `adc` command instead.
- Replace the "Wrote N bytes to EEPROM." print in `Setting.cpp` with `SETTINGS_SAVE`/`SETTINGS_LOAD`/`SETTINGS_DEFAULT` events.
- Delete the commented-out print in `AnalogComparator.cpp`.
- Add the read-only commands that expose the same data on demand: `sensor` (tip temperature, NTC temperature, VIN, iron current, Rtip, Rsupply), `adc [temp|ntc|current|vin]` (raw and converted per channel), `heater` (state, setpoint, target, heatfactor, PWM duty), `pid` (Pterm/Iterm/Dterm, Kp/Ki/Kd), `settings` (current EEPROM values).
- Floats are formatted by `include/CliFormat.h`, not by `%f`: the SAMD21 links newlib-nano without `_printf_float`, so `%f` in `vsnprintf` prints nothing.
- Flash and verify: the USB-CDC port stays silent while the heater runs, and each command returns the values the old prints showed.
- `pio run` passes.

### [x] M4 - W25Q64 storage (flash: yes)

- Add Adafruit_SPIFlashBase with `Adafruit_FlashTransport_SPI` on the default SPI pads (PA12/PB10/PB11) with FLASH_CS on PA20.
- Add LittleFS on top. On mount: blank-check the first sector; if blank (0xFF) or mount fails, format and mount again.
- `flash erase yes-really` erases one sector per main-loop pass and inhibits the heater output for the duration of the erase; `flash` reports the progress.
- CLI: `flash` (JEDEC ID, size, status), `flash info` (JEDEC ID, size, 64-bit unique ID, decoded status 1/2/3, ready), `flash read <addr> <len>` (capped at 256 bytes), `flash erase yes-really`, `fs` (mount state, total/used bytes), `fs ls [path]`, `fs cat <path>` (capped at 4 KiB), `fs write <path> <text>`, `fs rm <path> yes-really`, `fs format yes-really`.
- `flash info` needs commands the Adafruit transport cannot issue, because it has no dummy-byte support: a small raw-SPI helper sends the unique ID command (0x4B, four dummy bytes) and the status 3 read (0x15). Status 1 and 2 come from the library.
- The block-protect and lock bits are reported as `none`, `partial` or `all` rather than as an address range, because the exact range depends on the datasheet table of the fitted device.
- `pio run` passes.
- Verified on V2.2 hardware: `flash` reports JEDEC `0xEF4017` and 8 MiB, `fs` reports mounted with 8192 bytes used, a file written with `fs write` survives a reboot, `flash erase` runs to completion in the background while the CLI stays responsive, and `fs format` restores the volume afterwards.
- Verified on V2.2 hardware: `flash info` reports unique ID `D164B8424F2D3021`, identical before and after a reboot, status 1/2/3 all `0x00` (no busy, no write enable, no block protect, no lock, drive strength 100%), and the filesystem still reads and writes normally after the raw-SPI reads.
- The heater output is inhibited for the duration of the erase. The inhibit path is not yet exercised with the heater on: the Heat controller has no local `heater on` command until M7, so the bench test runs with the heater off.

### [x] M5 - Settings as JSON (flash: yes)

- Add ArduinoJson 7. Serialize the seven settings fields to `/settings.json` (compact) with camelCase keys identical to the `SettingClass` members; deserialize on boot, and a missing key falls back to its default.
- Double-buffer the settings file: copy `/settings.json` to `/settings.json.bak`, then write the new `/settings.json`. On boot `/settings.json` wins; `/settings.json.bak` is used only when the primary is missing or invalid.
- Keep the debounced save behavior of the current SettingClass.
- Remove the CRC/union versioning code, `Setting_t`, `ROM_START`, and the `CRC` and `EepromAbstractionWire` dependencies. `IoAbstraction` and `TaskManagerIO` leave with them.
- No EEPROM migration: the first boot after this milestone starts from defaults.
- Flash and verify: change a setting over the minion bus, confirm it persists across a reboot, confirm defaults appear on a blank filesystem.
- CLI: extend `settings` to the JSON store: `settings` (dump all keys, camelCase, so a printed key is valid `settings set` input), `settings get <key>`, `settings set <key> <value>` (writes and saves), `settings save`, `settings load`, `settings defaults yes-really`, `settings file` (active file, sizes, validity of both buffers).
- `pio run` passes.
- Verified on V2.2 hardware: `settings` dumps the seven camelCase keys, `settings set targetTemperatureHeater 400` writes a 240-byte `/settings.json` that survives a reboot, `settings file` reports both buffers valid, and `fs format` plus a reboot returns the defaults with a `SETTINGS_DEFAULT` event.
- Verified on V2.2 hardware: a corrupt primary falls back to `/settings.json.bak` on boot (`SETTINGS_LOAD`), and the next save keeps the valid backup instead of copying the corrupt primary over it.
- Verified on V2.2 hardware: `heat setpoint 150` from the ESP32 writes 423.15 K over the minion bus and the value survives a Heat controller reboot.

### [ ] M6 - ADS1115-only measurement (flash: yes)

- Read TEMP, NTC, IRON_CURRENT, VIN from ADS1115 channels AIN0-AIN3 over `Wire`, conversion-ready signaled by ADC_RDY (PA06) interrupt.
- Delete AnalogComparator; SHAKE is read as a digital GPIO input in M8.
- Restore the SAMD21 internal ADC as a read-only diagnostic. It reads the same analog nets as the ADS1115 and never feeds the control loop. The 2.23 V reference clips the NTC below about 10 C; the ADS1115 covers that range.
- Bench fact: with a cold tip, the TEMP net swings between about 0.005 V and 0.23 V with a mean of about 0.08 V (20 C). The ADS1115 and the SAMD21 internal ADC show the same swing, so the swing is in the analog signal and not in the ADC read path. The cause is investigated with a scope on the hardware bench.
- Flash and verify the tip temperature and heater current readings against the values the current firmware reports, and confirm the heating cycle stays stable.
- CLI: extend `adc`/`sensor` to the ADS1115-only path: `adc` (all four channels, raw and converted, with the SAMD21 ADC value and absolute difference in V in the second and third column), `adc raw`, `adc <temp|ntc|current|vin>`, `sensor` (tip temperature, NTC temperature, heater current, supply voltage), `sensor offset [<volts>]` (get/set `thermocoupleInputOffsetVoltage`).
- `pio run` passes.

### [ ] M7 - Minion protocol and HEAT_INT (flash: yes, joint with ESP32 M6)

- Define and document the I2C minion register protocol the ESP32 uses to read temperatures and control the heater.
- Document the HEAT_INT (PA14) flag bitmask semantics: the SAMD21 raises the line and latches a 32-bit flag word, the ESP32 reads REGISTER_INTERRUPT to fetch and clear it.
- Implement link supervision in the SAMD21: a watchdog timer restarts only on a read of REGISTER_HEATER_STATE; when no heartbeat arrives within 5 seconds, the SAMD21 forces `heaterState` to OFF and raises the `INT_LINK_TIMEOUT` flag. Heartbeats restart supervision, so heating resumes automatically when the ESP32 returns.
- Document the link supervision contract in the README: the ESP32 reads REGISTER_HEATER_STATE at least once per 5 seconds as its heartbeat, and `INT_LINK_TIMEOUT` signals a lost link.
- This milestone lands together with milestone M6 of the Main processor plan, in one integration session on the bench: both processors get the new register map in the same test run, so the register map never disagrees between the two firmwares.
- CLI: extend `heater`/`pid`/`setpoint`; add `minion` (address, last transaction, latched interrupt flags, HEAT_INT asserted), `minion read <reg> [len]`, `minion write <reg> <hex...> yes-really` (bench-only, bypasses the protocol), `regs` (full register map with names, addresses, types, current values), `int` (latched flags), `int clear`, `link` (last heartbeat age, timeout flag, forced-off count), `link timeout [<ms>]`, `heater on|off`, `setpoint [<celsius>]`, `pid kp|ki|kd <value>`.
- `pio run` passes.

### [ ] M8 - SHAKE standby (flash: yes, nice-to-have, lowest priority)

- Read SHAKE (PA04/PB09) as a digital GPIO input with a small software debounce and switch the heater to a standby temperature on lack of motion; return to the setpoint on motion.
- The standby temperature and timeout are settings in `/settings.json`.
- CLI: `shake` (raw level, debounced state, time since last motion, standby active), `shake standby [<celsius>]`, `shake timeout [<ms>]`.
- `pio run` passes.
