# Main processor

The main processor is the ESP32 controller of the soldering iron. It drives the touchscreen, the audio output and the network services, and it controls the heat controller over the I2C minion bus.

## Hardware

- ESP32 (esp32dev) with 16 MB flash.
- ILI9341 320x240 display with an XPT2046 touch controller, rotation 1.
- MAX98357A I2S audio amplifier.
- Heat controller on I2C address 0x2A, HEAT_INT on GPIO35.

## Heat link

The main processor is the I2C master. It reads register 0x11 once per second as the link heartbeat. The heat controller forces the heater off when the heartbeat stops for 5 seconds. The register map matches the heat controller README. The main processor refuses register writes when the firmware version differs.

## Display

The carousel has four screens:

| Screen  | Content                                                                                              |
| ------- | ---------------------------------------------------------------------------------------------------- |
| Home    | Tip temperature, setpoint, PWM duty, heater state, link state, power and setpoint buttons, AP button |
| Network | WiFi state, SSID, IP address, RSSI, AP button                                                        |
| Info    | Firmware version, session uptime, lifetime total, boot count                                         |
| Touch   | Touch calibration and touch test buttons                                                             |

The setpoint changes in 5 C steps. A held button repeats after 500 ms at 150 ms intervals. The setpoint range is 0-450 C.

## Audio

The MAX98357A plays tones at 16 kHz through I2S. The gain is 6, 9, 12 or 15 dB.

## Network

The device connects to one of two configured WiFi networks. When no network is available it starts an access point with a captive portal. The hostname and mDNS name are `iot12-xxxxxx`, where `xxxxxx` is the last three bytes of the station MAC address.

The web server serves the page and these routes:

| Route           | Method | Content                                                          |
| --------------- | ------ | ---------------------------------------------------------------- |
| `/api/status`   | GET    | WiFi state, IP, RSSI, portal state, uptime, heap                 |
| `/api/device`   | GET    | Name, version, git commit, build time, board, chip               |
| `/api/config`   | GET    | Configured network SSIDs                                         |
| `/api/screenshot` | GET  | RGB565 screenshot of the display                                 |
| `/api/session`  | GET    | Whether the request carries a valid session cookie               |
| `/api/login`    | POST   | Check the AP password and set the session cookie                 |
| `/api/logout`   | POST   | Clear the session cookie in the browser                          |
| `/api/regenerate` | POST | Replace the AP password and rotate the session token             |
| `/api/wifi`     | POST   | Save the network list and reconnect; requires unlock in station mode |
| `/api/ota`      | POST   | Upload a firmware image; requires unlock in station mode         |
| `/api/stream`   | GET    | Server-sent events with the live state                           |

The write routes are open in portal mode, because the client already proved the AP password to join the access point. In station mode the page unlocks with the AP password and receives an HttpOnly session cookie. The cookie is valid until the device reboots. Logout clears the cookie in that browser. Regenerating the AP password rotates the session token, so every other browser is locked out.

The live state carries tip, setpoint, NTC, duty, supply voltage, iron current, heater state, link state, WiFi state, SSID, IP, RSSI, portal state, uptime and heap.

## Storage

The lifetime counters and the touch calibration are stored in NVS. The network list and the access point password are stored in NVS through the web configuration store. The access point password also unlocks the webui in station mode.

## Serial CLI

The serial port runs at 115200 baud. The CLI provides system, log, heat, net, display, ui, audio, screen and stats commands. Use `help` for the command list.

## Build

```
pio run
```

The build generates `include/WebContent.h` from the `webui` directory and adds the short git commit as `GIT_COMMIT`.
