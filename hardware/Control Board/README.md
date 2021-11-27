# Assembly instructions Control Board

## Solder

Solder components in the following order.
Use the [interactive BOM](bom/ibom.html) for values and positions of the components.

### Integrated circuit

U1.
U2.
U3, U4.
U5.
U6.
U7.

### Transistor

Q1, Q2.
Q3.

### Diode

D1, D2, D3, D4, D12.
D5.
D6, D7, D8.
D9.
D10.
D11.

### Inductor

L1.
L2.

### Resistor

R1.
R10, R11, R12.
R20.
R21.
R30.
R31, R32, R33, R34.
R40.
R52.
R50, R51, R53, R54, R55, R56, R57, R58, R59, R82.
R60, R61, R62, R63. **Read schematic notes about these resistors.**
R64.
R70.
R71, R72.
R80, R81, R83.

### Capacitor

C1, C2.
C10.
C20, C21.
C30, C31, C32, C33, C34, C35, C36, C37, C38.
C40, C41.
C50, C60, C61.
C70, C71, C73.

### Crystal

Y1.

## Clean PCB

[Clean your printed circuit board](http://letmegooglethat.com/?q=how+to+clean+flux+after+soldering). It should be squeaky clean, no flux residue or other crud should remain.

### Switch, back of PCB

SW2.

### Connector, back of PCB

J10.

### Connector, front of PCB

A2: 2 pcs of 1x8pin 2.54mm female headers.

### Connector, back of PCB

A1: 2 pcs of 1x19pin 2.54mm female headers.
J9, J10. 2x5pin 2.54mm male headers.

### Fuse, back of PCB

## Clean PCB, again.

Yes, do it.

### Encoder, front of PCB

Solder SW1 and remove nut and washer.

### Module, front of PCB

A3. Use pin male headers. Also for SPK pins.

## Inspect

Inspect all solder joints. They should all be [nicely soldered](https://www.google.com/search?q=proper+solder+joint), no [solder bridges](https://www.google.com/search?q=picture+of+a+solder+bridge) should be present. Have a magnifying glass or (digital) microscope? Use it.

### GX12-5 connector

Solder some wires between a GX12-5 socket connector and the back of the pcb. On the back of the pcb there are a few pads labeled 1 to 5, solder them to pin 1 to 5 on the GX12-5 connector.

| Pin#  | Wire color    | Description   |
| --:   | --            | --            |
| 1     | blue          | shake         |
| 2     | white         | NTC           |
| 3     | green         | ESD           |
| 4     | black         | T12-          |
| 5     | red           | T12+          |

## Testing

Connect a (KSGER) T12 solder iron handle with the T12 tip firmly seated inside.

- Measure about 1MΩ between the tip of the iron and EARTH hole at the Power supply connection on the PCB.
- Now measure some values at the solder iron connector holes.\
The heater resistor between 4 (black) and 5 (red): 8Ω
- Switch over to measure mV. When you have not touched the handle and tip for half an hour you should read 0mV. Grab the tip with your hand. You should notice an increasing voltage, possibly a rise until 0.1mV. It is possible your voltmeter is not capable of measuring in the microvolt range. You can heat the tip with a flame for a few seconds to increase the voltage some more.
- The NTC between 4 (black) and 2 (white): 5KΩ@22°C, but that could vary slightly when not measuring at the same room temperature.
- Test the tilt sensor. Point iron up and measure between 1 (blue) and 3 (green): >2MΩ\
Point iron down and measure a low resistance: <100Ω

Solder a red wire to 24V and black wire to GND of the power supply connector holes, both on the back.\
Slowly increase power supply until 10V with max 50mA power. Measure voltage at C71. You should measure 5V. The current draw should be under 10mA.

Switch off power supply and mount the LCD display module and the ESP32 module.

Increase maximum power supply current to 250mA. Instantly increase voltage to 5V, the slowly to 10V, current should not exceed 150mA. When at 10V the current draw should be under 100mA.
You may increase voltage to 24V, consumed power should be under 40mA.

When all tests went well, start writing your own firmware. Or [flash my firmware](https://github.com/atoomnetmarc/IoT12-firmware)
