# Status

**Experimental. Do not replicate.**

**You should be present at all times when using this circuit in case something goes wrong.**

# IoT12-hardware
Schematic and PCB files of the IoT12 solder iron temperature controller for T12 tips.

My [Kicad library](https://github.com/atoomnetmarc/ATOOMNETKICAD) is used in this project.

Control Board:\
![](Control%20Board/IoT12%20Control%20Board-render.jpg)

Power Supply:\
![](Power%20Supply/IoT12%20Power%20Supply-render.jpg)

Display Adapter:\
![](Display%20Adapter/Display%20Adapter-render.jpg)

# Checklist

:ballot_box_with_check:Design schematic\
:ballot_box_with_check:Route V1 control board PCB\
:ballot_box_with_check:Slept a few days before ordering V1 control board PCB\
:ballot_box_with_check:Order V1 control board PCB\
:ballot_box_with_check:Made my first github repository\
:ballot_box_with_check:V1 control board PCB arrived\
:ballot_box_with_check:Solder V1 control board PCB and make assembly instructions\
:ballot_box_with_check:Observe that there is something wrong in schematic\
:ballot_box_with_check:Debug V1 control board PCB\
:ballot_box_with_check:Redesign control board PCB, add microcontroller\
:ballot_box_with_check:Order V2 control board PCB\
:ballot_box_with_check:Design power supply PCB\
:ballot_box_with_check:Order power supply PCB\
:ballot_box_with_check:V2 control board PCB and power supply PCB arrived\
:ballot_box_with_check:Solder V2 control board PCB and make assembly instructions\
:ballot_box_with_check:Solder V1 power supply PCB and make assembly instructions\
:black_square_button:Debug V2 control board PCB\
:ballot_box_with_check:Start making firmware\
:black_square_button:Finish first firmware version\
:black_square_button:...\
:black_square_button:profit

# Checklist V2.0 control PCB

Worky (after bodge fix)?

:ballot_box_with_check:SAMD21 debug LED\
:ballot_box_with_check:SAMD21 USB device\
:ballot_box_with_check:SAMD21 SWD\
:ballot_box_with_check:SAMD21 TP1/D0\
:ballot_box_with_check:SAMD21 TP2/D1\
:ballot_box_with_check:SAMD21 TP3/D9\
:ballot_box_with_check:SAMD21 TP4/D10\
:ballot_box_with_check:SAMD21 HEAT_INT\
:ballot_box_with_check:SAMD21 HEAT_PWM\
:ballot_box_with_check:SAMD21 I2C minion\
:ballot_box_with_check:SAMD21 I2S_GAIN0\
:ballot_box_with_check:SAMD21 I2S_GAIN1\
:ballot_box_with_check:SAMD21 I2S_SD\
:ballot_box_with_check:SAMD21 ADC_RDY\
:ballot_box_with_check:SAMD21 I2C ADC\
:ballot_box_with_check:SAMD21 I2C ADC NTC\
:ballot_box_with_check:SAMD21 I2C ADC TEMP\
:ballot_box_with_check:SAMD21 I2C ADC VIN\
:ballot_box_with_check:SAMD21 I2C ADC IRON_CURRENT\
:ballot_box_with_check:SAMD21 I2C EEPROM\
:ballot_box_with_check:SAMD21 ADC NTC\
:ballot_box_with_check:SAMD21 ADC TEMP\
:ballot_box_with_check:SAMD21 ADC VIN\
:ballot_box_with_check:SAMD21 ADC IRON_CURRENT\
:ballot_box_with_check:SAMD21 ADC AIN_COMP\
:ballot_box_with_check:SAMD21 AC AIN_COMP\
:ballot_box_with_check:ESP32 LCD_LED\
:ballot_box_with_check:ESP32 ROT_SW\
:ballot_box_with_check:ESP32 ROT_A\
:ballot_box_with_check:ESP32 ROT_B\
:ballot_box_with_check:ESP32 I2S\
:ballot_box_with_check:ESP32 I2C\
:ballot_box_with_check:ESP32 JTAG\
:ballot_box_with_check:ESP32 LCD\
:ballot_box_with_check:ESP32 HEAT_INT


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

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
