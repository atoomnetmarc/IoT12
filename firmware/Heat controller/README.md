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
|     0x00 | R   | 16-byte str |         |                |      | Firmware version              |
|     0x01 | R   | 4x uint32_t |         |                |      | SAMD21 128-bit serial number  |
|     0x10 | R   |    uint32_t |         |                |      | Interrupt reason              |
|     0x11 | RW  |     uint8_t |    0x00 | N              |      | Heating state                 |
|     0x12 | RW  |    uint32_t |    5000 | N              |   ms | Link supervision timeout      |
|     0x40 | R   |       float |         |                |    K | Current tip temperature       |
|     0x41 | R   |       float |         |                |    K | Ambient temperature           |
|     0x80 | RW  |       float |  373.15 | Y              |    K | Target tip temperature        |
|     0xA0 | RW  |       float |  0.0449 | Y              |      | Gain of Vin resistor divider  |
|     0xA1 | RW  |       float |   106.1 | Y              |      | Gain of temperature amplifier |
|     0xA2 | RW  |       float |    50.4 | Y              |      | Gain of current amplifier     |
|     0x90 | RW  |       float |   10000 | Y              |    Ω | NTC pullup resistor           |
|     0x91 | RW  |       float |   0.005 | Y              |    Ω | Heater shunt resistor         |
|     0xF8 | R   |    uint16_t |  0x1234 |                |      | A constant                    |
|     0xF9 | R   |       float |    3.14 |                |      | Mmmm, pi                      |

### 0x00: Firmware Version

A 16-byte NUL-terminated ASCII string in Calendar Versioning format `YYYY.MM.Micro`, for example `2026.10.1`.

- `YYYY`: year
- `MM`: month
- `Micro`: release counter within the month, reset to 1 when the month changes

The Heat controller and the Main processor must report the same version to cooperate. The master must:

1. Read this register during initialization
2. Compare the string with its own version
3. Refuse register writes when the strings differ

### 0x01: Serial Number

Unique 128-bit SAMD21 microcontroller serial number represented as four consecutive 32-bit words (little-endian order).

### 0x10: Interrupt Reason

32-bit register where reading clears the HEAT_INT pin and resets the register to 0.

#### Bit Definitions:

| Bit  | Name | Description                     |
| ---- | ---- | ------------------------------- |
| 31   | Z    | Heat controller rebooted        |
| 30   | Y    | Group A measurements available  |
| 29   | X    | Group B measurements available  |
| 28   | W    | Link timeout, heater forced off |
| 27-0 |      | Reserved (ignore if set)        |

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

A read of this register is the link heartbeat. See "Link Supervision" below.

### 0x12: Link Supervision Timeout

The time in milliseconds without a heartbeat before the heat controller forces the heater off. The default is 5000 ms. The value is not saved in NVRAM.

### 0x80: Target Tip Temperature

Maximum allowed value: 725K (451.85°C)

### 0xF8, 0xF9: Test Constants

Fixed values for verifying little-endian multibyte transfers:

- 0xF8: uint16_t (0x1234)
- 0xF9: float (π ≈ 3.14)

# Link Supervision

The heat controller owns heater safety. The master must read register 0x11 (heating state) at least once per 5 seconds. That read is the heartbeat: it restarts the supervision timer. No other register read or write restarts the timer.

When no heartbeat arrives within the timeout (register 0x12, default 5000 ms), the heat controller:

1. Forces the heating state to 0x00 (off).
2. Raises bit 28 (INT_LINK_TIMEOUT) in the interrupt register and asserts HEAT_INT.

The master reads register 0x10 to fetch and clear the flag, reports the lost link, and writes register 0x11 again after communication resumes. Heating resumes on that write. The two controllers cannot reset each other.

# Settings Storage

The registers marked "Saved in NVRAM" are stored in `/settings.json` on the LittleFS volume of the W25Q64 SPI flash. The target temperature is in K (kelvin), the same unit as register 0x80.

A save copies `/settings.json` to `/settings.json.bak` and then writes the new `/settings.json`. That means the backup is always the last completed save. On boot `/settings.json` is read first; `/settings.json.bak` is read only when the primary file is missing or invalid. A key that is missing from the file keeps its default.

# Fuse Configuration

BOD33 enabled with hysteresis at user level 0x25
