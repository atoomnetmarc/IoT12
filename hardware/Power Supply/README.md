*TBD for version V1.2*


# Warning

If you choose to replicate and use this PCB you are doing so on your own risk. It is very much possible to set your house on fire when something goes wrong or die when touching the wrong spots on the PCB.

When handling this PCB you are working with dangerous mains voltage ⚡. When touched it hurts the entire time you are dying ☠️. The bottom and top the PCB contain exposed tracks and screw terminals carrying deadly mains voltage.

In conclusion, do not use this PCB. Power the IoT12 control board with some other safe and certified power supply.

# Assembly instructions Power Supply

## Solder

Solder components in the following order.
Use the [interactive BOM](bom/ibom.html) for values and positions of the components.

## Testing

Place bottom of PCB on isolating material.

Connect leads of a mains cable to J1. Use the correct screw terminal for the correct mains line. If top of PCB is not clear enough, use labels on bottom of PCB and/or consult the PCB design files.

⚡ Connect the mains cable to the wall socket.

Notice the green LED lighting up. Now carefully measure the low voltage side on screw terminal J2. You should measure 24V volts.

Disconnect the mains cable from the wall socket. Notice that the green LED is still lit. It will stop emitting light after some unit of time. The capacitors in the Mean Well power supply still contain some chare and are discharging slowly.

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)