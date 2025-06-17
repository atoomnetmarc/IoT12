# I2C minion

The heat controller functions as an I2C minion to our ESP32 I2C master.

Minion I2C address: **0x2A**

The minion interrupts the master by setting the HEAT_INT-pin high. The master must examine why the minion wants the attention by reading the interrupt register before reading and writing any other registers.

## Registers

There are a few registers which can be read and written to in order to communicate with and configure the heat controller. Multibyte reads and writes are [little-endian](https://en.wikipedia.org/wiki/Endianness) format.

Writes are executed by writing the register value first and then the value of the register. In case of an uint32_t or [float](https://en.wikipedia.org/wiki/Single-precision_floating-point_format) write that means writing 5 bytes in one transaction.

A register read is executed by first writing a single byte that selects the register you want to read, then reading the byte(s) of that register in one transaction. In case of a read for an uint32_t or float that means reading 4 bytes.

Some registers are saved in NVRAM. That means once set the value earlier written will be remembered between power cycles and reboots.

Registers that have something to do with temperature are all in K (kelvin) and stored in a float.

Registers that store resistance are in Ω (ohm) and are also stored in a float.

Registers that store gain are in unitless factors and stored in a float.

The firmware version register is the only register that must be read. All other are somewhat optional and operate from carefully chosen defaults when not modified.

| Register | RW  |        Size | Default | Saved in NVRAM | Unit | Description                   |
| -------: | --- | ----------: | ------: | -------------- | ---: | ----------------------------- |
|     0x00 | R   |    uint32_t |         |                |      | Firmware version              |
|     0x01 | R   | 4x uint32_t |         |                |      | SAMD21 128-bit serial number  |
|     0x10 | R   |    uint32_t |         |                |      | Interrupt reason              |
|     0x11 | RW  |     uint8_t |    0x00 | N              |      | Heating state                 |
|     0x40 | R   |       float |         |                |    K | Current tip temperature       |
|     0x41 | R   |       float |         |                |    K | Ambient temperature           |
|     0x80 | RW  |       float |  373.15 | Y              |    K | Target tip temperature        |
|     0xA0 | RW  |       float |  0.0449 | Y              |      | Gain of Vin resistor divider  |
|     0xA1 | RW  |       float |   106.1 | Y              |      | Gain of temperature amplifier |
|     0xA2 | RW  |       float |    50.4 | Y              |      | Gain of current amplifier     |
|     0x90 | RW  |       float |   10000 | Y              |    Ω | NTC pullup resistor           |
|     0x91 | RW  |       float |   0.005 | Y              |    Ω | Heater shunt resistor         |
|     0xF0 | RW  |     uint8_t |    0x02 | Y              |      | I2S gain                      |
|     0xF1 | RW  |     uint8_t |    0x00 | N              |      | I2S enable                    |
|     0xF8 | R   |    uint16_t |  0x1234 |                |      | A constant                    |
|     0xF9 | R   |       float |    3.14 |                |      | Mmmm, pi                      |

### 0x00: Firmware Version

Stored as a 32-bit integer with the bit format:
`0bXXXXXXXXX XXYYYYYYYYY YYYYYYZZZZZZ`
Presented as "X.Y.Z" where:

- X: 11-bit major version (0-2047)
- Y: 15-bit minor version (0-32767)
- Z: 6-bit patch version (0-63)

Version range: 0.0.0 to 2047.32767.63
Follows [Semantic Versioning](https://semver.org/) principles:

- Major version (X) changes indicate breaking changes
- Minor version (Y) changes add backward-compatible features
- Patch version (Z) changes are backward-compatible bug fixes

The master must:

1. Read this register during initialization
2. Halt communication if major version is incompatible
3. Only allow register writes when compatible firmware is detected

### 0x01: Serial Number

Unique 128-bit SAMD21 microcontroller serial number represented as four consecutive 32-bit words (little-endian order).

### 0x10: Interrupt Reason

32-bit register where reading clears the HEAT_INT pin and resets the register to 0.

#### Bit Definitions:

| Bit  | Name | Description                    |
| ---- | ---- | ------------------------------ |
| 31   | Z    | Heat controller rebooted       |
| 30   | Y    | Group A measurements available |
| 29   | X    | Group B measurements available |
| 28-0 |      | Reserved (ignore if set)       |

#### Handling Requirements:

1. If bit Z is set:
   - Immediately read firmware version (0x00)
   - Ignore all other bits until version is verified
2. Process other bits only after firmware verification

#### Measurement Groups:

- **Group A**: Measurements taken with heater off
  (tip temperature, ambient temperature)
- **Group B**: Measurements taken with heater on
  (supply voltage, heater current)

### 0x11: Heating State

| Value | State |
| ----- | ----- |
| 0x00  | Off   |
| 0x01  | On    |

### 0x80: Target Tip Temperature

Maximum allowed value: 725K (451.85°C)

### 0xF0: I2S Gain

| Value | Gain |
| ----- | ---- |
| 0x02  | 9dB  |
| 0x03  | 12dB |
| 0x04  | 15dB |

### 0xF1: I2S Enable

| Value | State    |
| ----- | -------- |
| 0x00  | Disabled |
| 0x01  | Enabled  |

### 0xF8, 0xF9: Test Constants

Fixed values for verifying little-endian multibyte transfers:

- 0xF8: uint16_t (0x1234)
- 0xF9: float (π ≈ 3.14)

# Fuse Configuration

BOD33 enabled with hysteresis at user level 0x25
