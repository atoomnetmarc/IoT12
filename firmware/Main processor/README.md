# Main processor

The main processor is the ESP32 controller of the soldering iron. It drives the touchscreen, the audio output and the network services, and it controls the heat controller over the I2C minion bus.

## Hardware

- ESP32 (esp32dev) with 16 MB flash.
- ILI9341 320x240 display with an XPT2046 touch controller, rotation 1.
- MAX98357A I2S audio amplifier.
- Heat controller on I2C address 0x2A, HEAT_INT on GPIO35.

## Heat link

The main processor is the I2C master. It reads register 0x11 once per second as the link heartbeat. The heat controller forces the heater off when the heartbeat stops for 5 seconds. The register map matches the heat controller README. The main processor refuses register writes when the firmware version differs.

## Handle diagnosis

The main processor classifies the handle from the measurements that the heat controller publishes. The first match wins.

| Order | State | Condition | Line 1 | Line 2 |
| ----- | ----- | --------- | ------ | ------ |
| 1 | `LOSS` | `linkUp()` is false | `HW fault` | `Heat ctrl absent` |
| 2 | `NO_HANDLE` | `NTC_V > 2.5` | `No handle` | `Connect T12 handle` |
| 3 | `SUPPLY_OFF` | `VIN_OFF_V < 5` | `24V is off` | `Switch 24V on` |
| 4 | `NO_TIP` | `THERMOCOUPLE_MV / 1000 * gain > 1.5`, or heater on for 1 s AND `duty > 0.2` AND `IRON_CURRENT_ON_A < 0.1` | `No T12 tip` | `Press tip in firmly` |
| 5 | `OK` | none of the above | tip temperature | `SET`/`PWM` |

The thresholds come from the measurement log in the heat controller `MEASUREMENTS.md`. The thermocouple check works with the heater off. The current check waits 1 s after the heater turns on, because the heat controller refreshes `IRON_CURRENT_ON_A` one cycle later. The `sensor` command prints `HANDLE_STATE`.

## Display

The carousel has four screens:

| Screen  | Content                                                                                              |
| ------- | ---------------------------------------------------------------------------------------------------- |
| Home    | Tip temperature, setpoint, PWM duty, heater state, power and setpoint buttons, AP button. A handle fault replaces the tip value and the status rail with a two-line judgement. The action line is green except in the LOSS state. A fault is shown after it has been stable for one second. POWER may always stop the heater; it may only start it in the OK state. It is green while the heater is on, yellow while the target ramps down to standby, and red in the 24V-off state. It is disabled for starting in the other fault states. |
| Network | WiFi state, SSID, IP address, RSSI, AP button                                                        |
| Info    | Firmware version, session uptime, lifetime total, boot count                                         |
| Touch   | Touch calibration and touch test buttons                                                             |

The setpoint changes in 5 C steps. A held button repeats after 500 ms at 150 ms intervals. The setpoint range is 0-450 C.

The `display info` command prints the panel, bus, touch and backlight configuration, including the controller IDs read over SPI.

## Audio

The MAX98357A plays tones at 16 kHz through I2S. The gain is 6, 9, 12 or 15 dB.

## Network

The device connects to one of two configured WiFi networks. When no network is available it starts an access point with a captive portal. The hostname and mDNS name are `iot12-xxxxxx`, where `xxxxxx` is the last three bytes of the station MAC address.

The web server serves the page and these routes:

| Route           | Method | Content                                                          |
| --------------- | ------ | ---------------------------------------------------------------- |
| `/api/status`   | GET    | WiFi state, IP, RSSI, portal state, uptime, heap                 |
| `/api/device`   | GET    | Fixed ESP32 and heat controller data, fetched after a reboot     |
| `/api/config`   | GET    | Configured network SSIDs                                         |
| `/api/screenshot` | GET  | RGB565 screenshot of the display                                 |
| `/api/session`  | GET    | Whether the request carries a valid session cookie               |
| `/api/login`    | POST   | Check the AP password and set the session cookie                 |
| `/api/logout`   | POST   | Clear the session cookie in the browser                          |
| `/api/regenerate` | POST | Replace the AP password and rotate the session token             |
| `/api/wifi`     | POST   | Save the network list and reconnect; requires unlock in station mode |
| `/api/ota`      | POST   | Upload a firmware image; requires unlock in station mode         |
| `/api/source`   | GET    | Gzipped tar of the main processor and heat controller source     |
| `/api/stream`   | GET    | Server-sent events with the live state                           |

The write routes are open in portal mode, because the client already proved the AP password to join the access point. In station mode the page unlocks with the AP password and receives an HttpOnly session cookie. The cookie is valid until the device reboots. Logout clears the cookie in that browser. Regenerating the AP password rotates the session token, so every other browser is locked out.

The live state carries tip, setpoint, NTC, duty, iron current, the handle state, the eight handle measurements, heater state, link state, WiFi state, SSID, IP, RSSI, portal state, uptime, total uptime, boot count, reset reason, heap, ESP32 die temperature, the heat controller uptime, reset cause, die temperature and supply, the link counters, the PID terms and gains, the raw PWM values and the shake state. The page shows the handle state and the action in the Live card, and the measurements in the same card. The Device card shows the name, board, chip, uptime and heap. The Firmware card shows the version, git commit and build time. The Network card shows only network data. The Nerd stats card shows the fixed ESP32 and heat controller data and the live counters.

## Source download

The build packs the main processor and heat controller source trees into one gzipped tar archive and embeds it in the firmware. The Source card links to the GitHub repository and downloads the archive from `/api/source`. The archive is the source at build time, so it matches the running firmware only when the build came from a clean tree. The file name carries the product version and the git commit.

## Nerd stats

The Nerd stats card shows every counter and identity field that the two processors expose. The page fetches the fixed data from `/api/device` once and again when either uptime goes backwards, so a reboot of the ESP32 or the heat controller refreshes the identity fields without polling them every second. The live counters arrive in the state stream.

The heat controller registers that feed the card are listed in the heat controller README. The ESP32 side uses the Arduino-ESP32 `ESP` API, `temperatureRead()` for the die temperature and the lifetime counters from NVS.

## Storage

The lifetime counters and the touch calibration are stored in NVS. The network list and the access point password are stored in NVS through the web configuration store. The access point password also unlocks the webui in station mode.

## Serial CLI

The serial port runs at 115200 baud. The CLI provides system, log, heat, link, sensor, net, display, ui, audio, screen and stats commands. Use `help` for the command list. The `sensor` command prints the handle measurements that the heat controller publishes and the `HANDLE_STATE` judgement, or `LINK: LOSS` when the link is down.

## Build

```
pio run
```

The build generates `include/WebContent.h` from the `webui` directory, generates `include/SourceBundle.h` from the two source trees, and generates `include/GitCommit.h` with the short git commit.
