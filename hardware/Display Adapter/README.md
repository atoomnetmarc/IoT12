Render

![Display adapter rendering](Display%20Adapter-render.jpg)

# Soldering

Yes, do it now. Use the [BOM](bom/ibom.html) for identifying the components.

Results:

![Yeah for soldering the only component on the BOM.](solder%20header.jpg)

Stick a about 2.4mm high sticky pad on the bottom like this:

![I used a 4mm sticky pad.](stick-y%20pad%20onto.jpg)

Did you bridge J1 on the TFT pcb to enable 3.3V operation? Do that before soldering the adapter board onto the TFT.

Next, solder the adapter board onto the back of the TFT display:

![Soldered adapter board to back of TFT display.](solder%20adapter%20to%20tft.jpg)

# Pinout J1, Board-to-board

| Pin# | Description | Direction |
| ---- | ----------- | --------- |
| 1    | +3.3V       | Power     |
| 2    | TFT_DC      | Out       |
| 3    | GND         | Power     |
| 4    | TFT_SCLK    | Out       |
| 5    | GND         | Power     |
| 6    | TFT_MOSI    | Out       |
| 7    | GND         | Power     |
| 8    | TFT_MISO    | In        |
| 9    | GND         | Power     |
| 10   | TFT_CS      | Out       |
| 11   | TFT_LED     | Out       |
| 12   | TFT_RST     | Out       |
| 13   | TOUCH_CS    | Out       |
| 14   | TOUCH_IRQ   | In        |

# Pinout J2, Display

Connector to the ILI9341 2.8" touch TFT. Direction seen from the adapter board.

| Pin# | Description | Direction |
| ---- | ----------- | --------- |
| 1    | +3.3V       | Power     |
| 2    | GND         | Power     |
| 3    | TFT_CS      | Out       |
| 4    | TFT_RST     | Out       |
| 5    | TFT_DC      | Out       |
| 6    | TFT_MOSI    | Out       |
| 7    | TFT_SCLK    | Out       |
| 8    | TFT_LED     | Out       |
| 9    | TFT_MISO    | In        |
| 10   | TFT_SCLK    | Out       |
| 11   | TOUCH_CS    | Out       |
| 12   | TFT_MOSI    | Out       |
| 13   | TFT_MISO    | In        |
| 14   | TOUCH_IRQ   | In        |

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
