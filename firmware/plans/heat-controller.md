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
- Measurement source of truth is the ADS1115 only. All four analog channels are read over the HEAT I2C bus. The SAMD21 internal ADC analog paths are removed. The ADS1115 sample timing fits the 20 ms measurement window of the heating cycle; this is proven by the working measurement code on the current hardware.
- I2C topology: `Wire` (sercom3, PA22/PA23) is the master bus for the ADS1115. A custom `TwoWire` instance on sercom0 (PA08/PA09) is the I2C minion bus to the ESP32. The expansion bus (PA16/PA17) stays untouched.
- Settings are stored as JSON (ArduinoJson) on the external W25Q64 SPI flash.
- Storage stack: Adafruit_SPIFlash + LittleFS on the W25Q64. When the flash is detected as blank (all 0xFF) it is formatted automatically. Partitioning beyond the LittleFS volume is deferred until a feature needs it.
- No EEPROM emulation in internal flash.
- The pin map and the heater PWM output move in one milestone, so no firmware build exists that compiles against the V2.2 pin defines while still driving the V1 pins. That milestone is compile-only: the minion bus still runs on sercom2 (PA14/PA15) until the I2C topology milestone, and PA14 becomes HEAT_INT in the pin map, so the two moves must both be in place before the build is flashed.
- The I2S registers are removed in M2; the ESP32 stops using them in its M5/M6. The remaining minion protocol change (link supervision, INT_LINK_TIMEOUT) lands in the same integration step as the matching ESP32 firmware change, so the two processors never disagree about the register map.
- Link supervision: the SAMD21 forces `heaterState` to OFF when no heartbeat arrives within 5 seconds, and raises the `INT_LINK_TIMEOUT` interrupt flag. The heartbeat is an explicit read of REGISTER_HEATER_STATE by the ESP32 at least once per 5 seconds; only that read restarts the SAMD21 watchdog, other I2C traffic does not. Heating resumes when the ESP32 resumes heartbeating. The two MCUs cannot reset each other; the SAMD21 owns heater safety.
- SHAKE is a digital signal: the handle line is a 100k pullup with a 100 pF RC filter (10 us time constant), read as a plain GPIO input with a small software debounce. The analog comparator is not used on V2.2.
- Settings persistence: double-buffered `/settings.json` and `/settings.json.bak` on LittleFS. Each save writes the inactive file completely, then renames it over the active one; on boot the newest valid file wins, and the other is a fallback if one is corrupt.
- SHAKE switches the heater to a standby temperature on lack of motion; this is the lowest-priority nice-to-have and is scheduled last.

## Test strategy

- Every milestone is flashed and functionally tested on the V2.2 hardware; `pio run` passing is the entry ticket, not the exit criterion.
- The ESP32 firmware on the minion bus is the test partner. The ESP32 firmware that exists at the start of this plan speaks the same minion protocol and the same HEAT_INT semantics, so it is a valid test partner for every milestone up to and including M6.
- M7 changes the register map together with the ESP32 firmware; from M7 onward the updated ESP32 firmware is the test partner.
- Each milestone below carries a flash marker: `flash: yes` means the build is safe and useful to test on V2.2 hardware, `flash: no` means compile-only because the build does not yet drive the V2.2 pins correctly.

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
- Compile-only: the minion bus still runs on sercom2 (PA14/PA15) while PA14 becomes HEAT_INT, so this build is not flashed onto V2.2 hardware.

### [ ] M3 - I2C topology (flash: yes)

- Keep `Wire` (sercom3, PA22/PA23) as the ADS1115 master bus; no variant changes needed.
- Add a custom `TwoWire` instance on sercom0 for PA08/PA09 and move WireMinion (I2C minion to the ESP32) onto it, following the instantiation pattern in the core `Wire.cpp`.
- Remove `I2C_ADDRESS_EEPROM` (the EEPROM is gone).
- Flash and verify the minion bus with the existing ESP32 firmware: register reads (version, temperatures) and REGISTER_INTERRUPT fetch-and-clear over HEAT_INT.
- `pio run` passes.

### [ ] M4 - W25Q64 storage (flash: yes)

- Add Adafruit_SPIFlash with `Adafruit_FlashTransport_SPI` on the default SPI pads (PA12/PB10/PB11) with FLASH_CS on PA20.
- Add LittleFS on top. On mount: blank-check the first sector; if blank (0xFF) or mount fails, format and mount again.
- Flash and verify the flash JEDEC ID is readable over USB serial and that a file survives a power cycle.
- `pio run` passes.

### [ ] M5 - Settings as JSON (flash: yes)

- Add ArduinoJson. Serialize the existing settings fields to `/settings.json` (compact), deserialize on boot; missing keys fall back to defaults.
- Double-buffer the settings file: write `/settings.json.bak` completely, then rename it over `/settings.json`; on boot load the newest valid file, fall back to the other one if it is corrupt.
- Keep the debounced save behavior of the current SettingClass.
- Remove the CRC/union versioning code and the EepromAbstractionWire dependency.
- Flash and verify: change a setting over the minion bus, confirm it persists across a reboot, confirm defaults appear on a blank filesystem.
- `pio run` passes.

### [ ] M6 - ADS1115-only measurement (flash: yes)

- Read TEMP, NTC, IRON_CURRENT, VIN from ADS1115 channels AIN0-AIN3 over `Wire`, conversion-ready signaled by ADC_RDY (PA06) interrupt.
- Delete AnalogComparator; SHAKE is read as a digital GPIO input in M8.
- Flash and verify the tip temperature and heater current readings against the values the current firmware reports, and confirm the heating cycle stays stable.
- `pio run` passes.

### [ ] M7 - Minion protocol and HEAT_INT (flash: yes, joint with ESP32 M6)

- Define and document the I2C minion register protocol the ESP32 uses to read temperatures and control the heater.
- Document the HEAT_INT (PA14) flag bitmask semantics: the SAMD21 raises the line and latches a 32-bit flag word, the ESP32 reads REGISTER_INTERRUPT to fetch and clear it.
- Implement link supervision in the SAMD21: a watchdog timer restarts only on a read of REGISTER_HEATER_STATE; when no heartbeat arrives within 5 seconds, the SAMD21 forces `heaterState` to OFF and raises the `INT_LINK_TIMEOUT` flag. Heartbeats restart supervision, so heating resumes automatically when the ESP32 returns.
- Document the link supervision contract in the README: the ESP32 reads REGISTER_HEATER_STATE at least once per 5 seconds as its heartbeat, and `INT_LINK_TIMEOUT` signals a lost link.
- This milestone lands together with milestone M6 of the Main processor plan, in one integration session on the bench: both processors get the new register map in the same test run, so the register map never disagrees between the two firmwares.
- `pio run` passes.

### [ ] M8 - SHAKE standby (flash: yes, nice-to-have, lowest priority)

- Read SHAKE (PA04/PB09) as a digital GPIO input with a small software debounce and switch the heater to a standby temperature on lack of motion; return to the setpoint on motion.
- The standby temperature and timeout are settings in `/settings.json`.
- `pio run` passes.
