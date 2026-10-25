# Assembly instructions

Use the [interactive BOM](bom/ibom.html) to identify values and positions of the components.

Solder all small 0805 SMD resistors and capacitors.
Read special instructions for R60, R61 and R62, R63 in the schematic.

Measure all resistors and check if they are all the correct resistance.

Solder rest of SMD components.

Clean PCB.

Inspect PCB with microscope for shorted pins (in ICs). Correct when found.

Solder all through-hole components.

Clean PCB.

You now have something like this:

![](IoT12%20Control%20Board-no%20modules.jpg)

Apply current limited 5V to GND and +5V on J4. Almost no current should flow.

Apply current limited 3.3V to GND and 3V3 on J6. Almost no current should flow.

# Done

Done!

# Pinout J1, Board-to-board Display

Connector to the Display Adapter. Direction seen from the control board.

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

# Pinout J2, Speaker

Spoken soldering instructions for the visually impaired.

| Pin # | Description |
| ----- | ----------- |
| 1     | Loudspeaker |
| 2     | Loudspeaker |

# Pinout J4, Board-to-board Power

Goes to power board.

| Pin # | Description        |
| ----- | ------------------ |
| 1     | 24V switched power |
| 2     | 5V always on       |
| 3     | GND                |
| 4     | 24V_ON             |

# Pinout J5, T12 soldering iron

| Pin # | Description                      |
| ----- | -------------------------------- |
| 1     | SHAKE_IN                         |
| 2     | NTC_IN                           |
| 3     | ESD, connected with 100KΩ to GND |
| 4     | IRON\_-                          |
| 5     | IRON\_+                          |

# Pinout J6, Expansion I/O

Expansion connector, wired to the SAMD21 (EXP_D5, EXP_D7, EXP_SCL, EXP_SDA). Currently unused.

| Pin # | Description |
| ----- | ----------- |
| 1     | EXP_D5      |
| 2     | EXP_D7      |
| 3     | EXP_SCL     |
| 4     | EXP_SDA     |
| 5     | +3.3V       |
| 6     | GND         |

# Pinout A1, ESP32

The wireless brains.

| GPIO # | Description | Direction |
| ------ | ----------- | --------- |
| EN     | ESP32_RESET | In        |
| IO32   | TFT_LED     | Out       |
| IO33   | I2S_SD      | Out       |
| IO25   | I2S_BCK     | Out       |
| IO26   | I2S_WS      | Out       |
| IO21   | ESP32_SDA   | Bidir     |
| IO22   | ESP32_SCL   | Out       |
| IO14   | I2S_GAIN1   | Out       |
| IO13   | I2S_GAIN0   | Out       |
| IO15   | TFT_CS      | Out       |
| IO2    | TFT_DC      | Out       |
| IO4    | TFT_RST     | Out       |
| IO5    | TOUCH_CS    | Out       |
| IO18   | TFT_SCLK    | Out       |
| IO19   | TFT_MISO    | In        |
| IO23   | TFT_MOSI    | Out       |
| IO27   | I2S_DATA    | Out       |
| IO34   | TOUCH_IRQ   | In        |
| IO35   | HEAT_INT    | In        |

# Pinout A3, SAMD21

Heater controller

| GPIO # | Description  | Direction |
| ------ | ------------ | --------- |
| RESET  | SAMD21_RESET | In        |
| PA02   | NTC          | In        |
| PB08   | TEMP         | In        |
| PB09   | SHAKE        | In        |
| PA05   | VIN          | In        |
| PA07   | IRON_CURRENT | In        |
| PA14   | HEAT_INT     | Out       |
| PA15   | EXP_D5       | Bidir     |
| PA20   | FLASH_CS     | Out       |
| PA21   | EXP_D7       | Bidir     |
| PA06   | ADC_RDY      | In        |
| PA07   | 24V_ON       | Out       |
| PA18   | HEAT_PWM     | Out       |
| PA22   | HEAT_SDA     | Bidir     |
| PA23   | HEAT_SCL     | Out       |
| PA08   | ESP32_SDA    | Bidir     |
| PA09   | ESP32_SCL    | In        |
| PA16   | EXP_SDA      | Bidir     |
| PA17   | EXP_SCL      | Out       |
| PA12   | MISO         | In        |
| PB11   | SCK          | Out       |
| PB10   | MOSI         | Out       |

# Pinout U2, ADS1115

16b I2C ADC, I2C address 0x48, on the HEAT I2C bus.

| Pin # | Pin name  | Description  | Direction |
| ----- | --------- | ------------ | --------- |
| 1     | ADDR      | GND          | In        |
| 2     | ALERT/RDY | ADC_RDY      | Out       |
| 3     | GND       | NEAR_ADC_GND | In        |
| 4     | AIN0      | TEMP         | In        |
| 5     | AIN1      | NTC          | In        |
| 6     | AIN2      | IRON_CURRENT | In        |
| 7     | AIN3      | VIN          | In        |
| 8     | VDD       | +3.3V        | In        |
| 9     | SDA       | HEAT_SDA     | Bidir     |
| 10    | SCL       | HEAT_SCL     | In        |

# Changelog

**Control board PCB version V2.2**

- Removed debug LED
- Removed 5110 LCD
- Removed rotary encoder
- Added header for 2.8" ILI9341 320x240 touch TFT
- Added relay for switching 24V
- Moved power supply circuitry to power supply PCB
- Removed small EEPROM
- Added SPI flash
- Added expansion header
- Layed out PCB again to fit on bottom lid of enclosure

**Control board PCB version V2.1**

- Swapped SDA/SCL I2C minion pins on heater controller.
- Swapped SDA/SCL I2C master pins on heater controller.
- Renamed PWM_HEAT to HEAT_PWM.
- Re-added MAX809S voltage monitor to keep ESP32 in reset if 3.3V is too low.
- Dimensioned F1 to 50% of Amax of T12-tip, see notes in schematic.
- Connected Vin ADC to after F1.
- Moved USB connector and reset button of heater controller.

**Control board PCB version V2.0**

- Replace safety features by microcontroller.
- Heating control now done by microcontroller.
- Changed shape and layout of pcb.

**Control board PCB version V1.1**

- Added explanation to schematic about using matched resistors for the instrumentation amplifiers.
- Changed gain resistors of the instrumentation amplifiers as to keep Vout under 1V at full scale input.
- Added series diode to output of voltage monitor.
- Lowered value of C2-C5 from 10nF to 1nF.
- Added D9 and D10.
- Changed value of R9 from 100R to 1k
- Added circuit to shutdown heating in case of overheating tip.

**Control board PCB version V1.0**

- First version that got made into PCB form.

# Eratta

**Control board PCB version V2.2**

No known erratum.

**Control board PCB version V2.1**

- It seems that after some use the 5110 LCD module has unreliable contacts. The LCD should be replaced by something else. PCB will need to be redesigned.
- A single occurrence of incorrect and fluctuating temperatures has been observed. Possibly caused by arcs in the power switch during power on. The power switch needs te be replaced by something else. Also the firmware needs to be made aware somehow of the situation and shutdown heating.

**Control board PCB version V2.0**

- I2C to heater controller not working. SDA/SCL are swapped. Swap them on the PCB near R55 and R56 to make the ESP32 communicate to the heater controller.
- A theme is developing :unamused:. I2C from the heater controller to the I2C EEPROM and ADC is not working. SDA/SCL are swapped. Swap them between R53, R54 and the heater controller on the bottom where the SDA/SCL traces run next to each other.
- ESP32 will lockup if power supply rise time is too slow.

**Control board PCB version V1.1**

- Overheating protection circuit not very stable. No fix.

**Control board PCB version V1.0**

- Silkscreen error in 'Solder iron connector'-block. '5, red, +.' and '4, black, ground.' are switched resulting in negative voltages for positive temperatures. Switch leads will solve this. Corrected in next PCB version.
- Gain of instrumentation amplifiers not correct.
- Programming by USB is impossible.
- Measuring temperature takes too long (20ms).
