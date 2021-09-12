EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 5 5
Title "IoT12-hardware output"
Date "2021-09-12"
Rev "V2.1"
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L power:GND #PWR0171
U 1 1 5FF2D4E1
P 1750 3750
F 0 "#PWR0171" H 1750 3500 50  0001 C CNN
F 1 "GND" H 1755 3577 50  0000 C CNN
F 2 "" H 1750 3750 50  0001 C CNN
F 3 "" H 1750 3750 50  0001 C CNN
	1    1750 3750
	1    0    0    -1  
$EndComp
Wire Wire Line
	1750 2750 1750 3100
Wire Wire Line
	1750 3400 1750 3750
Connection ~ 1750 2750
Wire Wire Line
	1750 2750 1950 2750
Wire Notes Line
	5850 550  5850 2800
Wire Notes Line
	5850 2800 10000 2800
Wire Notes Line
	10000 2800 10000 550 
Wire Notes Line
	10000 550  5850 550 
$Comp
L power:GND #PWR0169
U 1 1 60001267
P 8600 2500
F 0 "#PWR0169" H 8600 2250 50  0001 C CNN
F 1 "GND" H 8605 2327 50  0000 C CNN
F 2 "" H 8600 2500 50  0001 C CNN
F 3 "" H 8600 2500 50  0001 C CNN
	1    8600 2500
	1    0    0    -1  
$EndComp
Wire Wire Line
	8600 2500 8600 2000
$Comp
L power:+3.3V #PWR0163
U 1 1 60004C09
P 8600 900
F 0 "#PWR0163" H 8600 750 50  0001 C CNN
F 1 "+3.3V" H 8615 1073 50  0000 C CNN
F 2 "" H 8600 900 50  0001 C CNN
F 3 "" H 8600 900 50  0001 C CNN
	1    8600 900 
	1    0    0    -1  
$EndComp
Wire Wire Line
	8600 900  8600 1000
Text Notes 5900 750  0    118  ~ 0
Display
Text GLabel 6800 2150 0    50   Input ~ 0
LCD_LED
Wire Wire Line
	8200 1300 6800 1300
Wire Wire Line
	8200 1600 6800 1600
Wire Wire Line
	8200 1700 6800 1700
Text GLabel 6800 1150 0    50   Input ~ 0
LCD_~CE
Text GLabel 6800 1300 0    50   Input ~ 0
LCD_~RST
Text GLabel 6800 1450 0    50   Input ~ 0
LCD_DATA-~CMD
Text GLabel 6800 1600 0    50   Input ~ 0
LCD_DIN
Text GLabel 6800 1700 0    50   Input ~ 0
LCD_CLK
Wire Wire Line
	6800 1450 7700 1450
Wire Wire Line
	7700 1450 7700 1400
Wire Wire Line
	7700 1400 8200 1400
Wire Wire Line
	8200 1200 7700 1200
Wire Wire Line
	7700 1200 7700 1150
Wire Wire Line
	7700 1150 6800 1150
Wire Wire Line
	8200 1800 7700 1800
Wire Wire Line
	7700 1800 7700 1950
$Comp
L power:GND #PWR0168
U 1 1 60114E68
P 7700 2500
F 0 "#PWR0168" H 7700 2250 50  0001 C CNN
F 1 "GND" H 7705 2327 50  0000 C CNN
F 2 "" H 7700 2500 50  0001 C CNN
F 3 "" H 7700 2500 50  0001 C CNN
	1    7700 2500
	1    0    0    -1  
$EndComp
Wire Wire Line
	7700 2500 7700 2350
Wire Notes Line
	550  4100 5750 4100
Wire Notes Line
	5750 4100 5750 550 
Wire Notes Line
	5750 550  550  550 
$Comp
L power:GND #PWR0170
U 1 1 6020ABD9
P 1350 3750
F 0 "#PWR0170" H 1350 3500 50  0001 C CNN
F 1 "GND" H 1355 3577 50  0000 C CNN
F 2 "" H 1350 3750 50  0001 C CNN
F 3 "" H 1350 3750 50  0001 C CNN
	1    1350 3750
	1    0    0    -1  
$EndComp
Wire Wire Line
	1350 2900 1350 2750
Wire Wire Line
	1350 2750 1750 2750
$Comp
L power:GND #PWR0172
U 1 1 60357CA7
P 2700 3750
F 0 "#PWR0172" H 2700 3500 50  0001 C CNN
F 1 "GND" H 2705 3577 50  0000 C CNN
F 2 "" H 2700 3750 50  0001 C CNN
F 3 "" H 2700 3750 50  0001 C CNN
	1    2700 3750
	1    0    0    -1  
$EndComp
Wire Wire Line
	2700 3750 2700 2950
Wire Wire Line
	2700 2450 2700 2550
Wire Wire Line
	2700 2150 2700 2000
Wire Wire Line
	3300 2000 3050 2000
Connection ~ 2700 2000
Wire Wire Line
	2700 2000 2700 1600
$Comp
L power:+24V #PWR0164
U 1 1 60373F3B
P 2700 1000
F 0 "#PWR0164" H 2700 850 50  0001 C CNN
F 1 "+24V" H 2715 1173 50  0000 C CNN
F 2 "" H 2700 1000 50  0001 C CNN
F 3 "" H 2700 1000 50  0001 C CNN
	1    2700 1000
	1    0    0    -1  
$EndComp
Wire Wire Line
	2700 1000 2700 1300
$Comp
L power:+24V #PWR0166
U 1 1 6037B770
P 3600 1000
F 0 "#PWR0166" H 3600 850 50  0001 C CNN
F 1 "+24V" H 3615 1173 50  0000 C CNN
F 2 "" H 3600 1000 50  0001 C CNN
F 3 "" H 3600 1000 50  0001 C CNN
	1    3600 1000
	1    0    0    -1  
$EndComp
Wire Wire Line
	3600 1000 3600 1800
$Comp
L power:GND #PWR0173
U 1 1 6038A739
P 3600 3750
F 0 "#PWR0173" H 3600 3500 50  0001 C CNN
F 1 "GND" H 3605 3577 50  0000 C CNN
F 2 "" H 3600 3750 50  0001 C CNN
F 3 "" H 3600 3750 50  0001 C CNN
	1    3600 3750
	1    0    0    -1  
$EndComp
$Comp
L power:Earth #PWR0175
U 1 1 60399CE4
P 4200 3750
F 0 "#PWR0175" H 4200 3500 50  0001 C CNN
F 1 "Earth" H 4200 3600 50  0001 C CNN
F 2 "" H 4200 3750 50  0001 C CNN
F 3 "~" H 4200 3750 50  0001 C CNN
	1    4200 3750
	1    0    0    -1  
$EndComp
$Comp
L power:+24V #PWR0165
U 1 1 603AB6E0
P 3050 1000
F 0 "#PWR0165" H 3050 850 50  0001 C CNN
F 1 "+24V" H 3065 1173 50  0000 C CNN
F 2 "" H 3050 1000 50  0001 C CNN
F 3 "" H 3050 1000 50  0001 C CNN
	1    3050 1000
	1    0    0    -1  
$EndComp
Wire Wire Line
	3050 1600 3050 2000
Connection ~ 3050 2000
Wire Wire Line
	3050 2000 2700 2000
Wire Wire Line
	3600 2200 3600 2500
Wire Wire Line
	5150 2650 4250 2650
Wire Wire Line
	4250 2650 4250 2500
Wire Wire Line
	4250 2500 3600 2500
Connection ~ 3600 2500
Wire Wire Line
	3600 2500 3600 2600
$Comp
L power:+24V #PWR0167
U 1 1 603E5FB5
P 4250 1000
F 0 "#PWR0167" H 4250 850 50  0001 C CNN
F 1 "+24V" H 4265 1173 50  0000 C CNN
F 2 "" H 4250 1000 50  0001 C CNN
F 3 "" H 4250 1000 50  0001 C CNN
	1    4250 1000
	1    0    0    -1  
$EndComp
Wire Wire Line
	4250 1850 4250 1000
Wire Wire Line
	4250 2150 4250 2500
Connection ~ 4250 2500
Text Notes 600  750  0    118  ~ 0
Heater power
Text GLabel 4200 2650 0    50   Output ~ 0
IRON_+
Wire Wire Line
	4250 2650 4200 2650
Connection ~ 4250 2650
Wire Wire Line
	3600 2900 3600 3750
Wire Wire Line
	4500 3200 4500 3100
Wire Wire Line
	4500 2850 5150 2850
Wire Wire Line
	4500 3500 4500 3650
$Comp
L power:GND #PWR0176
U 1 1 6061AAE8
P 4500 3750
F 0 "#PWR0176" H 4500 3500 50  0001 C CNN
F 1 "GND" H 4505 3577 50  0000 C CNN
F 2 "" H 4500 3750 50  0001 C CNN
F 3 "" H 4500 3750 50  0001 C CNN
	1    4500 3750
	1    0    0    -1  
$EndComp
$Comp
L Transistor_BJT:BC817 Q2
U 1 1 5FF33F94
P 2600 2750
F 0 "Q2" H 2804 2796 50  0000 L CNN
F 1 "MMBT2222" H 2804 2705 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 2800 2675 50  0001 L CIN
F 3 "" H 2600 2750 50  0001 L CNN
	1    2600 2750
	1    0    0    -1  
$EndComp
$Comp
L Transistor_FET-extra:AOD403 Q3
U 1 1 5FF2B8DD
P 3500 2000
F 0 "Q3" H 3704 1954 50  0000 L CNN
F 1 "AOD403" H 3704 2045 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:TO-252-2" H 3700 1925 50  0001 L CIN
F 3 "" V 3500 2000 50  0001 L CNN
	1    3500 2000
	1    0    0    1   
$EndComp
$Comp
L Device:R R1
U 1 1 605F934E
P 4500 3350
F 0 "R1" H 4570 3396 50  0000 L CNN
F 1 "0R005" H 4570 3305 50  0000 L CNN
F 2 "Resistor_SMD:R_2512_6332Metric_Pad1.52x3.35mm_HandSolder" V 4430 3350 50  0001 C CNN
F 3 "~" H 4500 3350 50  0001 C CNN
	1    4500 3350
	1    0    0    -1  
$EndComp
$Comp
L Device:D_Schottky D5
U 1 1 603E637A
P 4250 2000
F 0 "D5" H 4250 2217 50  0000 C CNN
F 1 "SS14" H 4250 2126 50  0000 C CNN
F 2 "Diode_SMD:D_SMA" H 4250 2000 50  0001 C CNN
F 3 "~" H 4250 2000 50  0001 C CNN
	1    4250 2000
	0    1    1    0   
$EndComp
$Comp
L Device:D_Schottky D8
U 1 1 603BC8D1
P 3600 2750
F 0 "D8" H 3600 2967 50  0000 C CNN
F 1 "SS34" H 3600 2876 50  0000 C CNN
F 2 "Diode_SMD:D_SMA" H 3600 2750 50  0001 C CNN
F 3 "~" H 3600 2750 50  0001 C CNN
	1    3600 2750
	0    1    1    0   
$EndComp
$Comp
L Diode:BZX84Cxx D9
U 1 1 603ABB73
P 3050 1450
F 0 "D9" V 3004 1530 50  0000 L CNN
F 1 "BZX84C18" V 3095 1530 50  0000 L CNN
F 2 "Diode_SMD:D_SOT-23_ANK" H 3050 1450 50  0001 C CNN
F 3 "~" H 3050 1450 50  0001 C CNN
	1    3050 1450
	0    1    1    0   
$EndComp
$Comp
L Device:R R58
U 1 1 6035EC09
P 2700 1450
F 0 "R58" H 2770 1496 50  0000 L CNN
F 1 "4k7" H 2770 1405 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 2630 1450 50  0001 C CNN
F 3 "~" H 2700 1450 50  0001 C CNN
	1    2700 1450
	1    0    0    -1  
$EndComp
$Comp
L Device:R R59
U 1 1 6020AA0C
P 1350 3050
F 0 "R59" H 1420 3096 50  0000 L CNN
F 1 "4k7" H 1420 3005 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 1280 3050 50  0001 C CNN
F 3 "~" H 1350 3050 50  0001 C CNN
	1    1350 3050
	1    0    0    -1  
$EndComp
$Comp
L Device:LED D11
U 1 1 60208E90
P 1350 3450
F 0 "D11" V 1389 3332 50  0000 R CNN
F 1 "Red" V 1298 3332 50  0000 R CNN
F 2 "LED_SMD:LED_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 1350 3450 50  0001 C CNN
F 3 "~" H 1350 3450 50  0001 C CNN
	1    1350 3450
	0    -1   -1   0   
$EndComp
$Comp
L Display_Graphic-extra:Nokia_5110_display A2
U 1 1 5FFFC9C9
P 8600 1500
F 0 "A2" H 8930 1591 50  0000 L CNN
F 1 "2x Conn_01x09_Female" H 8930 1500 50  0000 L CNN
F 2 "Display-extra:Nokia_5110_display_module" H 8600 900 50  0001 C CNN
F 3 "" H 9000 1200 50  0001 C CNN
F 4 "Nokia_5110_display" H 8930 1409 50  0000 L CNN "Description"
	1    8600 1500
	1    0    0    -1  
$EndComp
$Comp
L Device:R R71
U 1 1 5FF2D2ED
P 1750 3250
F 0 "R71" H 1820 3296 50  0000 L CNN
F 1 "100k" H 1820 3205 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 1680 3250 50  0001 C CNN
F 3 "~" H 1750 3250 50  0001 C CNN
	1    1750 3250
	1    0    0    -1  
$EndComp
$Comp
L Device:R R34
U 1 1 5FF21B0C
P 2100 2750
F 0 "R34" V 1893 2750 50  0000 C CNN
F 1 "1k" V 1984 2750 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 2030 2750 50  0001 C CNN
F 3 "~" H 2100 2750 50  0001 C CNN
	1    2100 2750
	0    1    1    0   
$EndComp
$Comp
L Device:R R33
U 1 1 6035F335
P 2700 2300
F 0 "R33" H 2770 2346 50  0000 L CNN
F 1 "1k" H 2770 2255 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 2630 2300 50  0001 C CNN
F 3 "~" H 2700 2300 50  0001 C CNN
	1    2700 2300
	1    0    0    -1  
$EndComp
$Comp
L Device:R R82
U 1 1 60060621
P 3900 3350
F 0 "R82" H 3830 3304 50  0000 R CNN
F 1 "1M" H 3830 3395 50  0000 R CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 3830 3350 50  0001 C CNN
F 3 "~" H 3900 3350 50  0001 C CNN
	1    3900 3350
	-1   0    0    1   
$EndComp
$Comp
L Device:R R83
U 1 1 6039BF91
P 4200 3350
F 0 "R83" H 4270 3396 50  0000 L CNN
F 1 "1M" H 4270 3305 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 4130 3350 50  0001 C CNN
F 3 "~" H 4200 3350 50  0001 C CNN
	1    4200 3350
	1    0    0    -1  
$EndComp
Wire Wire Line
	4200 3200 4200 3100
$Comp
L power:GND #PWR0174
U 1 1 6006090C
P 3900 3750
F 0 "#PWR0174" H 3900 3500 50  0001 C CNN
F 1 "GND" H 3905 3577 50  0000 C CNN
F 2 "" H 3900 3750 50  0001 C CNN
F 3 "" H 3900 3750 50  0001 C CNN
	1    3900 3750
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x01 J5
U 1 1 60160AA6
P 5350 2650
F 0 "J5" H 5430 2692 50  0001 L CNN
F 1 "~" H 5430 2646 50  0000 L CNN
F 2 "TestPoint:TestPoint_THTPad_D2.0mm_Drill1.0mm" H 5350 2650 50  0001 C CNN
F 3 "~" H 5350 2650 50  0001 C CNN
	1    5350 2650
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x01 J3
U 1 1 601619FF
P 5350 2750
F 0 "J3" H 5430 2792 50  0001 L CNN
F 1 "~" H 5430 2746 50  0000 L CNN
F 2 "TestPoint:TestPoint_THTPad_D2.0mm_Drill1.0mm" H 5350 2750 50  0001 C CNN
F 3 "~" H 5350 2750 50  0001 C CNN
	1    5350 2750
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x01 J4
U 1 1 60161B01
P 5350 2850
F 0 "J4" H 5430 2892 50  0001 L CNN
F 1 "~" H 5430 2801 50  0000 L CNN
F 2 "TestPoint:TestPoint_THTPad_D2.0mm_Drill1.0mm" H 5350 2850 50  0001 C CNN
F 3 "~" H 5350 2850 50  0001 C CNN
	1    5350 2850
	1    0    0    -1  
$EndComp
Wire Wire Line
	4200 3500 4200 3750
Wire Wire Line
	3900 3500 3900 3750
Wire Wire Line
	3900 3200 3900 3100
Wire Wire Line
	3900 3100 4200 3100
Connection ~ 4200 3100
Wire Wire Line
	4200 3100 4200 2750
Text Label 4500 2750 0    50   ~ 0
ESD
$Comp
L Device:Net-Tie_2 NT5
U 1 1 604DCD37
P 4650 3650
F 0 "NT5" H 4650 3739 50  0000 C CNN
F 1 "~" H 4650 3740 50  0000 C CNN
F 2 "NetTie-extra:NetTie-2_SMD_Pad0.25mm" H 4650 3650 50  0001 C CNN
F 3 "~" H 4650 3650 50  0001 C CNN
	1    4650 3650
	1    0    0    -1  
$EndComp
Wire Wire Line
	4550 3650 4500 3650
Connection ~ 4500 3650
Wire Wire Line
	4500 3650 4500 3750
Text GLabel 4800 3650 2    50   Output ~ 0
NEAR_RSENSE_-
Wire Wire Line
	1350 3200 1350 3300
Wire Wire Line
	1350 3600 1350 3750
Wire Wire Line
	4750 3650 4800 3650
$Comp
L Device:Net-Tie_2 NT4
U 1 1 6014262E
P 4650 3100
F 0 "NT4" H 4650 3189 50  0000 C CNN
F 1 "~" H 4650 3190 50  0000 C CNN
F 2 "NetTie-extra:NetTie-2_SMD_Pad0.25mm" H 4650 3100 50  0001 C CNN
F 3 "~" H 4650 3100 50  0001 C CNN
	1    4650 3100
	1    0    0    -1  
$EndComp
Text GLabel 4800 3100 2    50   Output ~ 0
NEAR_RSENSE_+
Wire Wire Line
	4550 3100 4500 3100
Connection ~ 4500 3100
Wire Wire Line
	4500 3100 4500 2850
Wire Wire Line
	4750 3100 4800 3100
Wire Notes Line
	550  4100 550  550 
Wire Wire Line
	3050 1300 3050 1000
Text Label 4500 2850 0    50   ~ 0
IRON_-
Wire Wire Line
	4200 2750 5150 2750
Wire Wire Line
	2250 2750 2400 2750
Text GLabel 1200 2750 0    50   Input ~ 0
HEAT_PWM
Wire Wire Line
	1200 2750 1350 2750
Connection ~ 1350 2750
$Comp
L Transistor_BJT:BC817 Q1
U 1 1 602DC02C
P 7600 2150
F 0 "Q1" H 7804 2196 50  0000 L CNN
F 1 "MMBT2222" H 7804 2105 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 7800 2075 50  0001 L CIN
F 3 "" H 7600 2150 50  0001 L CNN
	1    7600 2150
	1    0    0    -1  
$EndComp
$Comp
L Device:R R32
U 1 1 602DD9AD
P 7100 2150
F 0 "R32" V 6893 2150 50  0000 C CNN
F 1 "1k" V 6984 2150 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 7030 2150 50  0001 C CNN
F 3 "~" H 7100 2150 50  0001 C CNN
	1    7100 2150
	0    1    1    0   
$EndComp
Wire Wire Line
	6800 2150 6950 2150
Wire Wire Line
	7250 2150 7400 2150
Text GLabel 2050 5400 0    50   Input ~ 0
I2S_BCK
Text GLabel 2050 5300 0    50   Input ~ 0
I2S_WS
Text GLabel 2050 5500 0    50   Input ~ 0
I2S_DATA
$Comp
L Module-extra:max98357 A3
U 1 1 60692413
P 2750 5500
F 0 "A3" H 2750 6081 50  0000 C CNN
F 1 "max98357" H 2750 5990 50  0000 C CNN
F 2 "Module-extra:max98357" H 2750 5500 50  0001 C CNN
F 3 "" H 2750 5500 50  0001 C CNN
	1    2750 5500
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR0177
U 1 1 60692F04
P 2750 4750
F 0 "#PWR0177" H 2750 4600 50  0001 C CNN
F 1 "+5V" H 2765 4923 50  0000 C CNN
F 2 "" H 2750 4750 50  0001 C CNN
F 3 "" H 2750 4750 50  0001 C CNN
	1    2750 4750
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0178
U 1 1 6069361E
P 2750 6200
F 0 "#PWR0178" H 2750 5950 50  0001 C CNN
F 1 "GND" H 2755 6027 50  0000 C CNN
F 2 "" H 2750 6200 50  0001 C CNN
F 3 "" H 2750 6200 50  0001 C CNN
	1    2750 6200
	1    0    0    -1  
$EndComp
Wire Wire Line
	2750 6200 2750 5900
Wire Wire Line
	2750 5100 2750 4750
Wire Wire Line
	2050 5300 2350 5300
Wire Wire Line
	2050 5400 2350 5400
Wire Wire Line
	2050 5500 2350 5500
NoConn ~ 3150 5450
NoConn ~ 3150 5550
Text GLabel 1150 5600 0    50   Input ~ 0
I2S_GAIN0
Text GLabel 2050 5700 0    50   Input ~ 0
I2S_SD
Wire Wire Line
	2350 5700 2050 5700
Wire Notes Line
	550  4200 3450 4200
Wire Notes Line
	3450 4200 3450 6500
Wire Notes Line
	3450 6500 550  6500
Wire Notes Line
	550  6500 550  4200
Text Notes 600  4400 0    118  ~ 0
Bleep bloop module
Text GLabel 1150 5700 0    50   Input ~ 0
I2S_GAIN1
$Comp
L Device:R R72
U 1 1 606A8C21
P 1400 5700
F 0 "R72" V 1515 5700 50  0000 C CNN
F 1 "100k" V 1606 5700 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 1330 5700 50  0001 C CNN
F 3 "~" H 1400 5700 50  0001 C CNN
	1    1400 5700
	0    1    1    0   
$EndComp
Wire Wire Line
	1150 5700 1250 5700
Wire Wire Line
	1150 5600 1650 5600
Wire Wire Line
	1550 5700 1650 5700
Wire Wire Line
	1650 5700 1650 5600
Connection ~ 1650 5600
Wire Wire Line
	1650 5600 2350 5600
Text Label 2750 2000 0    50   ~ 0
GATE
Text Notes 750  5450 0    50   ~ 0
Due to 3.3V gain drive\nvoltage and 5V VCC \nthe following gains \nare possible:\n9dB, 12dB and 15dB
$EndSCHEMATC
