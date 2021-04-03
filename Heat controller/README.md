# I2C minion

The heat controller functions as an I2C minion to our ESP32 I2C master.

Minion I2C address: **0x2A**

The minion interrupts the master by setting the HEAT_INT-pin high. The master must examine why the minion wants the attention by reading the interrupt register before reading and writing any other registers.

## Registers

There are a few registers which can be read and written to in order to communicate with and configure the heat controller. Multibyte reads and writes are MSB (most significant byte) first.

Writes are executed by writing the register value first and then the value of the register. In case of an uint32_t or [float](https://en.wikipedia.org/wiki/Single-precision_floating-point_format) write that means writing 5 bytes in one transaction.

A register read is executed by first writing a single byte that selects the register you want to read, then reading the byte(s) of that register in one transaction. In case of a read for an uint32_t or float that means reading 4 bytes.

Some registers are saved in NVRAM. That means once set the value earlier written will be remembered between power cycles and reboots.

Registers that have something to do with temperature are all in K (kelvin) and stored in a float.

Registers that store resistance are in Ω (ohm) and are also stored in a float.

Registers that store gain are in unitless factors and stored in a float.

The firmware version register is the only register that must be read. All other are somewhat optional and operate from carefully chosen defaults when not modified.

| Register  | RW    | Size      | Default   | Saved in NVRAM    | Unit  | Description                               |
| --:       | --    | --:       | --:       | --                | --:   | --                                        |
| 0x00      | R     | uint32_t  |           |                   |       | Firmware version                          |
| 0x08      | R     | 16B       |           |                   |       | SAMD21 128-bit serial number              |
| 0x10      | R     | uint32_t  |           |                   |       | Interrupt reason                          |
| 0x18      | RW    | uint8_t   | 0x00      | N                 |       | Heating state                             |
| 0x40      | RW    | float     | 300       | Y                 | K     | Tip temperature minimum.                  |
| 0x41      | RW    | float     | 725       | Y                 | K     | Tip temperature maximum.                  |
| 0x50      | RW    | float     | 550       | Y                 | K     | Tip temperature target.                   |
| 0x60      | R     | float     |           |                   | K     | Tip temperature.                          |
| 0x68      | R     | float     |           |                   | K     | Ambient temperature.                      |
| 0x80      | RW    | float     | 0.0449    | Y                 |       | Gain of Vin resistor divider.             |
| 0x81      | RW    | float     | 106.1     | Y                 |       | Gain of temperature amplifier.            |
| 0x82      | RW    | float     | 50.4      | Y                 |       | Gain of current amplifier.                |
| 0x90      | RW    | float     | 10000     | Y                 | Ω     | NTC pullup resistor.                      |
| 0xF0      | RW    | uint8_t   | 0x02      | Y                 |       | I2S gain.                                 |
| 0xF1      | RW    | uint8_t   | 0x00      | N                 |       | I2S enable.                               |

### 0x00

Firmware version in format 0bXXXXXXXX.XXXYYYYY.YYYYYYYY.YYZZZZZZ, presented as "X.Y.Z" where X is 11 bits wide, Y is 15 bits wide and Z is 6 bits wide. So minimum version is "0.0.0" and maximum is "2047.32767.63". The version number follows the [SemVer specification](https://semver.org/) loosely. In short that means that the master must be updated when X changes and may be updated when Y changes. This also means that as soon as the master detects an incompatible X, it must stop all communication with the heat controller in order to not cause any unwanted behavior. When a compatible firmware is detected register writes are allowed.

The master is encouraged to read the firmware version register on its boot.

All versions of the heat controller must include the firmware version register in uint32_t.

### 0x08

Unique 128-bit serial number of the SAMD21 microcontroller.

### 0x10

4 bytes of interrupt reasons. Reading the register resets the HEAT_INT-pin and the contents of the register to 0. Must not be read when HEAT_INT-pin is low.

Every bit of the 4 bytes indicate a condition which may be investigated further. There is one mandatory action on the Z-bit. When set this indicates a reboot of the heat controller and means that the firmware version must be checked, all other bits are to be ignored until the firmware version has been read.

Format 0bZxxxxxx.xxxxxxxxx.xxxxxxxxx.xxxxxxxxx

| Bit   | Description                   |
| --    | --                            |
| 32, Z | Reboot of heat controller.    |
| x     | Undefined, ignore if set.     |

All versions of the heat controller must include the interrupt reason register in uint32_t.

### 0x50

The maximum hardcoded temperature that the heat controller will accept is 725K (451.85°C).

### 0xF0

Allowed I2S gain values are: 0x02 (9dB), 0x03 (12dB) and 0x04 (15dB).

### 0xF1

Allowed I2S enable values are: 0x00 (disabled) and 0x01 (enabled).

# Fuses

Enable BOD33 with hysteresis on userlevel 0x25.

