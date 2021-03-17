# I2C minion

The heat controller functions as an I2C minion to our ESP32 I2C master.

Minion I2C address: **0x2A**

The minion interrupts the master by setting the HEAT_INT-pin high. The master must examine why the minion wants the attention by reading the interrupt register before reading and writing any other registers.

## Registers

There are a few registers which can be read and written to in order to communicate with and configure the heat controller. Multibyte reads and writes are MSB (most significant byte) first.

Writes are executed by writing the register value first and then the value of the register. In case of an uint32_t write that means writing 5 bytes in one transaction.

A register read is executed by first writing a single byte that selects the register you want to read, then reading the byte(s) of that register in one transaction. In case of a read for an uint32_t that means reading 4 bytes.

Some registers are saved in NVRAM. That means once set the value earlier written will be remembered between power cycles and reboots.

Registers that have something to do with temperature are all in mK (millikelvin) and transported in an uint32_t. That means that the range of temperature goes from 0K (or -272.15°C) to 4,294,967.295K (or 7,730,481.46°F) which is somewhat on the warmer side.

Registers that store resistance are in mΩ (milliohm) and are also stored in an uint32_t. Range: from 0Ω to about 4.2MΩ.

Registers that store gain are in unitless factors of 1/1000000 and stored in an uint32_t. Range from 0.000001 to 4294.967295. A value of 1000000 means gain '1', or no gain.

The firmware version register is the only register that must be read. All other are somewhat optional and operate from carefully chosen defaults when not modified.

| Register | RW | Size | Default | Saved in NVRAM | Unit | Description |
| -- | -- | -- | -- | -- | -- | -- |
| 0x00 | R | uint32_t | | | | Firmware version |
| 0x10 | R | uint32_t | | |  | Interrupt reason |
| 0x11 | RW | uint8_t | 0x00 | N | |  Heating state |
| 0x20 | RW | uint32_t | 300000 | Y | mK | Tip temperature minimum. |
| 0x21 | RW | uint32_t | 725000 | Y | mK | Tip temperature maximum. |
| 0x22 | RW | uint32_t | 550000 | Y | mK | Target tip temperature. |
| 0x23 | R | uint32_t | | | mK | Tip temperature. |
| 0x24 | R | uint32_t | | | mK | Ambient temperature. |
| 0x80 | RW | uint32_t | | Y | | Vin gain * 1/1000000. |
| 0x81 | RW | uint32_t | 106100000 | Y | | Temperature amplifier gain * 1/1000000. |
| 0x82 | RW | uint32_t | 50400000 | Y |  | Current amplifier gain * 1/1000000. |
| 0x90 | RW | uint32_t | 10000000 | Y | mΩ | NTC pullup resistor. |
| 0xD0 | RW | uint8_t | 0x02 | Y | |  I2S gain. |
| 0xD1 | RW | uint8_t | 0x00 | N | |  I2S enable. |

### 0x00

Firmware version in format 0bXXXXXXXX.XXXYYYYY.YYYYYYYY.YYZZZZZZ, presented as "X.Y.Z" where X is 11 bits wide, Y is 15 bits wide and Z is 6 bits wide. So minimum version is "0.0.0" and maximum is "2047.32767.63". The version number follows the [SemVer specification](https://semver.org/) loosely. In short that means that the master must be updated when X changes and may be updated when Y changes. This also means that as soon as the master detects an incompatible X, it must stop all communication with the heat controller in order to not cause any unwanted behavior. When a compatible firmware is detected register writes are allowed.

The master is encouraged to read the firmware version register on its boot.

All versions of the heat controller must include the firmware version register in uint32_t.

### 0x10

4 bytes of interrupt reasons. Reading the register resets the HEAT_INT-pin and the contents of the register to 0. Must not be read when HEAT_INT-pin is low.

Every bit of the 4 bytes indicate a condition which may be investigated further. There is one mandatory action on the Z-bit. When set this indicates a reboot of the heat controller and means that the firmware version must be checked, all other bits are to be ignored until the firmware version has been read.

Format 0bZxxxxxx.xxxxxxxxx.xxxxxxxxx.xxxxxxxxx

| Bit | Description |
| -- | -- |
| 32, Z | Reboot of heat controller. |
| x | Undefined, ignore if set. |

All versions of the heat controller must include the interrupt reason register in uint32_t.

### 0x22

The maximum hardcoded temperature that the heat controller will accept is 725K (451.85°C).

### 0xD0

Allowed I2S gain values are: 0x02 (9dB), 0x03 (12dB) and 0x04 (15dB).

### 0xD1

Allowed I2S enable values are: 0x00 (disabled) and 0x01 (enabled).

# Fuses

Enable BOD33 with hysteresis on userlevel 0x25.

