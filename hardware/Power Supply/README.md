![Finished Power Supply PCB](IoT12%20Power%20Supply.jpg)

This Power Supply PCB powers the IoT12 Control Board. It features a relay that can be activated by the Control Board to provide 24V power to the soldering iron. However, the relay can only be activated when the user switch on J2 is closed. This enables the user to power down the heating of the soldering iron independent of what the Control Board is able to, which is a nice feature of an internet connected soldering station.

# Warning

If you choose to replicate and use this PCB you are doing so on your own risk. It is very much possible to set your house on fire when something goes wrong or die when touching the wrong spots on the PCB.

When handling this PCB you are working with dangerous mains voltage ⚡. When touched it hurts the entire time you are dying ☠️. The bottom and top the PCB contain exposed tracks and screw terminals carry deadly mains voltage.

In conclusion, do not use this PCB.

# Assembly instructions Power Supply

Use the [interactive BOM](bom/ibom.html) to identify values and positions of the components.

Solder every SMD component. Begin with the smallest, end with the tallest.

Solder every through hole component. Begin with the smallest, end with the tallest However, do not solder the Mean Well power supply (PS1) module itself yet.

Clean the PCB.

You have now something that looks like this:

![PCB without PS1](IoT12%20Power%20Supply%20-%20without%20ps1.jpg)

Connect current limited (100mA) 24V on the outputs of where PS1 should be. Possibly you could connect the right leg of F1 to 24V. Current should be under 10mA. Now D4 should light up. Measure the 5V pin, it should measure about 5.2V.
Connect a jumper to J2. Connect 3V3 to gnd and the '+24V ON'-pin of J3. D5 should light up and you hear a relay click. Current increases to about 20mA.

All went well? Disconnect 24V and solder PS1.

Measure 1M between GND on J3 and Earth on J1. Measure no connection between GND on J3 and L and N on J1.

Place bottom of PCB on isolating material.

Connect leads of an unconnected grounded mains cable to J1. Use the correct screw terminal for the correct mains line. If top of PCB is not clear enough, use labels on bottom of PCB and/or consult the PCB design files.

Connect multimeter to right leg of F1 and GND to measure voltage.

⚡ Connect the mains cable to the wall socket.

LED D4 should light up again. Look, no touch, at multimeter. It should read 24V.

Disconnect the mains cable from the wall socket. Disconnect multimeter. Notice D4 is still lit. It will stop emitting light after some unit of time. The capacitors in the Mean Well power supply still contain some charge and are discharging slowly.

# Done

Done!

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)