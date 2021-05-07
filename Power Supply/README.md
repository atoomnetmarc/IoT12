# Assembly instructions Power Supply

## Solder

Solder components in the following order.
Use the [interactive BOM](bom/ibom.html) for values and positions of the components.

### Resistor


R2.

### Diode


D2.

### Screw terminal


J1, J2.

### Power supply


One off PS1, PS2 or PS3.

## Testing


⚡ ☠️ **Warning**! When testing you are  (mm|planck distances|inches) away from dangerous mains voltage. When touched it hurts the entire time you are dying. The bottom and top the PCB contain exposed tracks and screw terminals carrying mains voltage. Consult expert on how to handle mains voltages.

**You must use a wall socket that is protected by some kind of [Residual Current Device (RCD)](https://en.wikipedia.org/wiki/Residual-current_device)** that breaks the connection to mains voltage in the unfortunate event you touch live wires.

Place bottom of PCB on isolating material.

Connect leads of a mains cable to J1. Use the correct screw terminal for the correct mains line. If top of PCB is not clear enough, use labels on bottom of PCB and/or consult the PCB design files.

⚡ Connect the mains cable to the wall socket.

Notice the green LED lighting up. Now carefully measure the low voltage side on screw terminal J2. You should measure 24V volts (if you mounted a Mean Well MPM-90-24).

Disconnect the mains cable from the wall socket. Notice that the green LED is still lit. It will stop emitting light after some unit of time. The capacitors in the Mean Well power supply still contain some chare and are discharging slowly.

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)