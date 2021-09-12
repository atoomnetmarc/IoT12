EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 3 5
Title "IoT12-hardware power supply"
Date "2021-09-12"
Rev "V2.1"
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Wire Notes Line
	550  3700 7900 3700
Wire Notes Line
	7900 3700 7900 550 
Wire Notes Line
	7900 550  550  550 
Wire Notes Line
	550  550  550  3700
Text Notes 600  750  0    118  ~ 0
Power supply
Text Notes 2150 2600 0    50   ~ 0
VADC = \nVin*0.0449
Text Notes 700  1650 0    50   ~ 0
Max 24V DC
$Comp
L Connector_Generic:Conn_01x01 J8
U 1 1 602E39D1
P 950 1950
F 0 "J8" H 1030 1992 50  0001 L CNN
F 1 "~" H 1030 1946 50  0000 L CNN
F 2 "TestPoint:TestPoint_THTPad_D2.0mm_Drill1.0mm" H 950 1950 50  0001 C CNN
F 3 "~" H 950 1950 50  0001 C CNN
	1    950  1950
	-1   0    0    1   
$EndComp
$Comp
L Connector_Generic:Conn_01x01 J7
U 1 1 602E3855
P 950 1850
F 0 "J7" H 1030 1892 50  0001 L CNN
F 1 "~" H 1030 1846 50  0000 L CNN
F 2 "TestPoint:TestPoint_THTPad_D2.0mm_Drill1.0mm" H 950 1850 50  0001 C CNN
F 3 "~" H 950 1850 50  0001 C CNN
	1    950  1850
	-1   0    0    1   
$EndComp
$Comp
L Connector_Generic:Conn_01x01 J6
U 1 1 602E2EF7
P 950 1750
F 0 "J6" H 1030 1792 50  0001 L CNN
F 1 "~" H 1030 1746 50  0000 L CNN
F 2 "TestPoint:TestPoint_THTPad_D2.0mm_Drill1.0mm" H 950 1750 50  0001 C CNN
F 3 "~" H 950 1750 50  0001 C CNN
	1    950  1750
	-1   0    0    1   
$EndComp
Wire Wire Line
	2600 2500 2700 2500
Text GLabel 2700 2500 2    50   Output ~ 0
VIN
Wire Wire Line
	2600 2500 2600 2700
Connection ~ 2600 2500
Wire Wire Line
	2600 3000 2600 3300
Wire Wire Line
	2600 2300 2600 2500
$Comp
L power:GND #PWR0124
U 1 1 5FFC18CE
P 2600 3300
F 0 "#PWR0124" H 2600 3050 50  0001 C CNN
F 1 "GND" H 2605 3127 50  0000 C CNN
F 2 "" H 2600 3300 50  0001 C CNN
F 3 "" H 2600 3300 50  0001 C CNN
	1    2600 3300
	1    0    0    -1  
$EndComp
$Comp
L Device:R R52
U 1 1 5FFC159B
P 2600 2850
F 0 "R52" V 2393 2850 50  0000 C CNN
F 1 "4k7 1%" V 2484 2850 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 2530 2850 50  0001 C CNN
F 3 "~" H 2600 2850 50  0001 C CNN
	1    2600 2850
	-1   0    0    1   
$EndComp
$Comp
L Device:R R70
U 1 1 5FFC0DAF
P 2600 2150
F 0 "R70" V 2393 2150 50  0000 C CNN
F 1 "100k 1%" V 2484 2150 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 2530 2150 50  0001 C CNN
F 3 "~" H 2600 2150 50  0001 C CNN
	1    2600 2150
	-1   0    0    1   
$EndComp
$Comp
L Device:D_Schottky D6
U 1 1 5FF43BD7
P 2800 1750
F 0 "D6" H 2800 1967 50  0000 C CNN
F 1 "SS34" H 2800 1876 50  0000 C CNN
F 2 "Diode_SMD:D_SMA" H 2800 1750 50  0001 C CNN
F 3 "~" H 2800 1750 50  0001 C CNN
	1    2800 1750
	-1   0    0    1   
$EndComp
$Comp
L Device:CP C70
U 1 1 5FF4D49D
P 3100 2500
F 0 "C70" H 3218 2546 50  0000 L CNN
F 1 "100uF/35V" H 3218 2455 50  0000 L CNN
F 2 "Capacitor_SMD:CP_Elec_6.3x7.7" H 3138 2350 50  0001 C CNN
F 3 "~" H 3100 2500 50  0001 C CNN
	1    3100 2500
	1    0    0    -1  
$EndComp
$Comp
L Device:C C33
U 1 1 5FF5E11D
P 3750 2500
F 0 "C33" H 3865 2546 50  0000 L CNN
F 1 "100nF" H 3865 2455 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder" H 3788 2350 50  0001 C CNN
F 3 "~" H 3750 2500 50  0001 C CNN
	1    3750 2500
	1    0    0    -1  
$EndComp
$Comp
L Regulator_Switching:LM2596S-ADJ U5
U 1 1 5FF63972
P 4650 1850
F 0 "U5" H 4650 2217 50  0000 C CNN
F 1 "LM2596S-ADJ" H 4650 2126 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:TO-263-5_TabPin3" H 4700 1600 50  0001 L CIN
F 3 "http://www.ti.com/lit/ds/symlink/lm2596.pdf" H 4650 1850 50  0001 C CNN
	1    4650 1850
	1    0    0    -1  
$EndComp
$Comp
L Device:D_Schottky D7
U 1 1 5FF7180F
P 5500 2500
F 0 "D7" H 5500 2717 50  0000 C CNN
F 1 "SS34" H 5500 2626 50  0000 C CNN
F 2 "Diode_SMD:D_SMA" H 5500 2500 50  0001 C CNN
F 3 "~" H 5500 2500 50  0001 C CNN
	1    5500 2500
	0    1    1    0   
$EndComp
$Comp
L Device:CP C71
U 1 1 5FF7BA64
P 7200 2500
F 0 "C71" H 7318 2546 50  0000 L CNN
F 1 "100uF/35V" H 7318 2455 50  0000 L CNN
F 2 "Capacitor_SMD:CP_Elec_6.3x7.7" H 7238 2350 50  0001 C CNN
F 3 "~" H 7200 2500 50  0001 C CNN
	1    7200 2500
	1    0    0    -1  
$EndComp
$Comp
L Device:C C34
U 1 1 5FF82B2F
P 6000 2500
F 0 "C34" H 6115 2546 50  0000 L CNN
F 1 "100nF" H 6115 2455 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder" H 6038 2350 50  0001 C CNN
F 3 "~" H 6000 2500 50  0001 C CNN
	1    6000 2500
	1    0    0    -1  
$EndComp
$Comp
L Device:R R40
U 1 1 5FF8F1BC
P 6950 1750
F 0 "R40" V 6743 1750 50  0000 C CNN
F 1 "3k3" V 6834 1750 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 6880 1750 50  0001 C CNN
F 3 "~" H 6950 1750 50  0001 C CNN
	1    6950 1750
	0    1    1    0   
$EndComp
$Comp
L Device:R R31
U 1 1 5FF982CE
P 6650 2500
F 0 "R31" V 6443 2500 50  0000 C CNN
F 1 "1k" V 6534 2500 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 6580 2500 50  0001 C CNN
F 3 "~" H 6650 2500 50  0001 C CNN
	1    6650 2500
	-1   0    0    1   
$EndComp
$Comp
L Device:L L1
U 1 1 5FF78815
P 5750 1950
F 0 "L1" V 5662 1950 50  0000 C CNN
F 1 "68uH" V 5571 1950 50  0000 C CNN
F 2 "Inductor-fixed:L_12x12mm_H8mm" H 5750 1950 50  0001 C CNN
F 3 "~" H 5750 1950 50  0001 C CNN
	1    5750 1950
	0    -1   -1   0   
$EndComp
$Comp
L Device:Polyfuse F2
U 1 1 5FFB5DA0
P 2100 1750
F 0 "F2" V 2233 1750 50  0000 C CNN
F 1 "0.5A/30V" V 2324 1750 50  0000 C CNN
F 2 "Fuse:Fuse_Bourns_MF-RG400" H 2150 1550 50  0001 L CNN
F 3 "~" H 2100 1750 50  0001 C CNN
	1    2100 1750
	0    1    1    0   
$EndComp
$Comp
L Device:Polyfuse F1
U 1 1 5FFBAD5E
P 2100 1550
F 0 "F1" V 1875 1550 50  0000 C CNN
F 1 "*see schematic." V 1966 1550 50  0000 C CNN
F 2 "Fuse:Fuse_Bourns_MF-RG1000" H 2150 1350 50  0001 L CNN
F 3 "~" H 2100 1550 50  0001 C CNN
	1    2100 1550
	0    1    1    0   
$EndComp
Wire Wire Line
	1750 3050 1750 3300
Connection ~ 1750 3050
Wire Wire Line
	2050 3050 1750 3050
Wire Wire Line
	2050 3300 2050 3050
$Comp
L power:PWR_FLAG #FLG0107
U 1 1 60809C07
P 2050 3300
F 0 "#FLG0107" H 2050 3375 50  0001 C CNN
F 1 "PWR_FLAG" H 2050 3473 50  0000 C CNN
F 2 "" H 2050 3300 50  0001 C CNN
F 3 "~" H 2050 3300 50  0001 C CNN
	1    2050 3300
	-1   0    0    1   
$EndComp
Wire Wire Line
	3750 900  3750 1750
$Comp
L power:PWR_FLAG #FLG0104
U 1 1 606BBB91
P 3750 900
F 0 "#FLG0104" H 3750 975 50  0001 C CNN
F 1 "PWR_FLAG" H 3750 1073 50  0000 C CNN
F 2 "" H 3750 900 50  0001 C CNN
F 3 "~" H 3750 900 50  0001 C CNN
	1    3750 900 
	1    0    0    -1  
$EndComp
Wire Wire Line
	1750 1850 1750 3050
Wire Wire Line
	1150 1850 1750 1850
Wire Wire Line
	1150 1950 1250 1950
$Comp
L power:Earth #PWR0123
U 1 1 5FFC6BAD
P 1750 3300
F 0 "#PWR0123" H 1750 3050 50  0001 C CNN
F 1 "Earth" H 1750 3150 50  0001 C CNN
F 2 "" H 1750 3300 50  0001 C CNN
F 3 "~" H 1750 3300 50  0001 C CNN
	1    1750 3300
	1    0    0    -1  
$EndComp
Wire Wire Line
	2250 1550 2600 1550
Wire Wire Line
	1750 1750 1950 1750
Connection ~ 1750 1750
Wire Wire Line
	1750 1550 1950 1550
Wire Wire Line
	1750 1750 1750 1550
Wire Wire Line
	1150 1750 1750 1750
Wire Wire Line
	6650 3300 6650 2650
$Comp
L power:GND #PWR0130
U 1 1 5FF9B82B
P 6650 3300
F 0 "#PWR0130" H 6650 3050 50  0001 C CNN
F 1 "GND" H 6655 3127 50  0000 C CNN
F 2 "" H 6650 3300 50  0001 C CNN
F 3 "" H 6650 3300 50  0001 C CNN
	1    6650 3300
	1    0    0    -1  
$EndComp
Wire Wire Line
	6650 1750 6800 1750
Connection ~ 6650 1750
Wire Wire Line
	6650 2350 6650 1750
Wire Wire Line
	5150 1750 6650 1750
Wire Wire Line
	7200 1750 7200 1550
Connection ~ 7200 1750
Wire Wire Line
	7100 1750 7200 1750
Wire Wire Line
	7200 1550 7200 900 
Connection ~ 7200 1550
Wire Wire Line
	7550 1550 7200 1550
Wire Wire Line
	7550 900  7550 1550
$Comp
L power:PWR_FLAG #FLG0105
U 1 1 5FF8C7CF
P 7550 900
F 0 "#FLG0105" H 7550 975 50  0001 C CNN
F 1 "PWR_FLAG" H 7550 1073 50  0000 C CNN
F 2 "" H 7550 900 50  0001 C CNN
F 3 "~" H 7550 900 50  0001 C CNN
	1    7550 900 
	1    0    0    -1  
$EndComp
Connection ~ 7200 1950
Wire Wire Line
	7200 1950 7200 1750
$Comp
L power:+5V #PWR0121
U 1 1 5FF89996
P 7200 900
F 0 "#PWR0121" H 7200 750 50  0001 C CNN
F 1 "+5V" H 7215 1073 50  0000 C CNN
F 2 "" H 7200 900 50  0001 C CNN
F 3 "" H 7200 900 50  0001 C CNN
	1    7200 900 
	1    0    0    -1  
$EndComp
Wire Wire Line
	6000 3300 6000 2650
$Comp
L power:GND #PWR0129
U 1 1 5FF8536E
P 6000 3300
F 0 "#PWR0129" H 6000 3050 50  0001 C CNN
F 1 "GND" H 6005 3127 50  0000 C CNN
F 2 "" H 6000 3300 50  0001 C CNN
F 3 "" H 6000 3300 50  0001 C CNN
	1    6000 3300
	1    0    0    -1  
$EndComp
Wire Wire Line
	6000 1950 5900 1950
Connection ~ 6000 1950
Wire Wire Line
	6000 2350 6000 1950
Wire Wire Line
	7200 3300 7200 2650
$Comp
L power:GND #PWR0131
U 1 1 5FF7E241
P 7200 3300
F 0 "#PWR0131" H 7200 3050 50  0001 C CNN
F 1 "GND" H 7205 3127 50  0000 C CNN
F 2 "" H 7200 3300 50  0001 C CNN
F 3 "" H 7200 3300 50  0001 C CNN
	1    7200 3300
	1    0    0    -1  
$EndComp
Wire Wire Line
	7200 1950 6000 1950
Wire Wire Line
	7200 2350 7200 1950
Connection ~ 5500 1950
Wire Wire Line
	5600 1950 5500 1950
Wire Wire Line
	5500 2650 5500 3300
$Comp
L power:GND #PWR0128
U 1 1 5FF74781
P 5500 3300
F 0 "#PWR0128" H 5500 3050 50  0001 C CNN
F 1 "GND" H 5505 3127 50  0000 C CNN
F 2 "" H 5500 3300 50  0001 C CNN
F 3 "" H 5500 3300 50  0001 C CNN
	1    5500 3300
	1    0    0    -1  
$EndComp
Wire Wire Line
	5500 1950 5150 1950
Wire Wire Line
	5500 2350 5500 1950
Wire Wire Line
	4650 2200 4650 3300
Connection ~ 4650 2200
Wire Wire Line
	3900 2200 4650 2200
Wire Wire Line
	4650 2150 4650 2200
$Comp
L power:GND #PWR0127
U 1 1 5FF67F54
P 4650 3300
F 0 "#PWR0127" H 4650 3050 50  0001 C CNN
F 1 "GND" H 4655 3127 50  0000 C CNN
F 2 "" H 4650 3300 50  0001 C CNN
F 3 "" H 4650 3300 50  0001 C CNN
	1    4650 3300
	1    0    0    -1  
$EndComp
Wire Wire Line
	3900 1950 3900 2200
Wire Wire Line
	4150 1950 3900 1950
Connection ~ 3750 1750
Wire Wire Line
	3750 1750 4150 1750
Wire Wire Line
	3750 3300 3750 2650
$Comp
L power:GND #PWR0126
U 1 1 5FF61D6F
P 3750 3300
F 0 "#PWR0126" H 3750 3050 50  0001 C CNN
F 1 "GND" H 3755 3127 50  0000 C CNN
F 2 "" H 3750 3300 50  0001 C CNN
F 3 "" H 3750 3300 50  0001 C CNN
	1    3750 3300
	1    0    0    -1  
$EndComp
Wire Wire Line
	3750 1750 3750 2350
Connection ~ 3100 1750
Wire Wire Line
	3750 1750 3100 1750
Wire Wire Line
	3100 3300 3100 2650
$Comp
L power:GND #PWR0125
U 1 1 5FF5140D
P 3100 3300
F 0 "#PWR0125" H 3100 3050 50  0001 C CNN
F 1 "GND" H 3105 3127 50  0000 C CNN
F 2 "" H 3100 3300 50  0001 C CNN
F 3 "" H 3100 3300 50  0001 C CNN
	1    3100 3300
	1    0    0    -1  
$EndComp
Wire Wire Line
	3100 1750 3100 2350
Wire Wire Line
	3100 1750 2950 1750
Wire Wire Line
	3000 1350 3000 900 
Connection ~ 3000 1350
Wire Wire Line
	3350 1350 3000 1350
Wire Wire Line
	3350 900  3350 1350
$Comp
L power:PWR_FLAG #FLG0103
U 1 1 5FF40C51
P 3350 900
F 0 "#FLG0103" H 3350 975 50  0001 C CNN
F 1 "PWR_FLAG" H 3350 1073 50  0000 C CNN
F 2 "" H 3350 900 50  0001 C CNN
F 3 "~" H 3350 900 50  0001 C CNN
	1    3350 900 
	1    0    0    -1  
$EndComp
Wire Wire Line
	3000 1550 3000 1350
$Comp
L power:+24V #PWR0120
U 1 1 5FF3E5BB
P 3000 900
F 0 "#PWR0120" H 3000 750 50  0001 C CNN
F 1 "+24V" H 3015 1073 50  0000 C CNN
F 2 "" H 3000 900 50  0001 C CNN
F 3 "" H 3000 900 50  0001 C CNN
	1    3000 900 
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0122
U 1 1 5FF39571
P 1250 3300
F 0 "#PWR0122" H 1250 3050 50  0001 C CNN
F 1 "GND" H 1255 3127 50  0000 C CNN
F 2 "" H 1250 3300 50  0001 C CNN
F 3 "" H 1250 3300 50  0001 C CNN
	1    1250 3300
	1    0    0    -1  
$EndComp
Text Label 1300 1750 0    50   ~ 0
24V_DC
Text Label 3200 1750 0    50   ~ 0
24V_DC_SW
Wire Wire Line
	1250 1950 1250 3300
$Comp
L Power_Supervisor:TPS3839DBZ U7
U 1 1 604FCB6B
P 1650 5050
F 0 "U7" H 1520 5096 50  0000 R CNN
F 1 "MAX809S" H 1520 5005 50  0000 R CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 1650 5050 50  0001 C CNN
F 3 "" H 1650 5050 50  0001 C CNN
	1    1650 5050
	1    0    0    -1  
$EndComp
$Comp
L Diode:BAV99 D12
U 1 1 604FD0FA
P 2550 5250
F 0 "D12" H 2550 5373 50  0000 C CNN
F 1 "BAV99" H 2550 5464 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 2550 4750 50  0001 C CNN
F 3 "https://assets.nexperia.com/documents/data-sheet/BAV99_SER.pdf" H 2550 5250 50  0001 C CNN
	1    2550 5250
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR0136
U 1 1 60501446
P 1650 5700
F 0 "#PWR0136" H 1650 5450 50  0001 C CNN
F 1 "GND" H 1655 5527 50  0000 C CNN
F 2 "" H 1650 5700 50  0001 C CNN
F 3 "" H 1650 5700 50  0001 C CNN
	1    1650 5700
	1    0    0    -1  
$EndComp
$Comp
L power:+3V3 #PWR0179
U 1 1 60501A96
P 1650 4400
F 0 "#PWR0179" H 1650 4250 50  0001 C CNN
F 1 "+3V3" H 1665 4573 50  0000 C CNN
F 2 "" H 1650 4400 50  0001 C CNN
F 3 "" H 1650 4400 50  0001 C CNN
	1    1650 4400
	1    0    0    -1  
$EndComp
Wire Wire Line
	1650 4750 1650 4400
Wire Wire Line
	2550 5050 2100 5050
Wire Wire Line
	1650 5350 1650 5700
NoConn ~ 2250 5250
$Comp
L Device:R R12
U 1 1 60506A5D
P 3300 5250
F 0 "R12" V 3093 5250 50  0000 C CNN
F 1 "100R" V 3184 5250 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 3230 5250 50  0001 C CNN
F 3 "~" H 3300 5250 50  0001 C CNN
	1    3300 5250
	0    -1   -1   0   
$EndComp
Wire Wire Line
	2850 5250 3150 5250
Text GLabel 3700 5250 2    50   Output ~ 0
ESP32_~RESET
Wire Wire Line
	3450 5250 3700 5250
Wire Notes Line
	550  3800 550  6100
Wire Notes Line
	550  6100 4900 6100
Wire Notes Line
	4900 6100 4900 3800
Wire Notes Line
	4900 3800 550  3800
Text Notes 600  4000 0    118  ~ 0
Voltage monitor
Text Notes 650  1350 0    50   ~ 0
*F1: fuse must be <=50% of amperage of T12 tip.\nFuse Vmax must exceed Vsupply.\nPower supply must be >=100% of amperage of T12 tip.\nMPM-90-24: 1.50A (JK60-150)\nMPM-30-15: 0.90A\nMPM-20-12: 0.75A
Wire Wire Line
	2250 1750 2650 1750
Wire Wire Line
	2600 2000 2600 1550
Connection ~ 2600 1550
Wire Wire Line
	2600 1550 3000 1550
$Comp
L Device:R R73
U 1 1 6071D566
P 2100 5450
F 0 "R73" V 1893 5450 50  0000 C CNN
F 1 "100K" V 1984 5450 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 2030 5450 50  0001 C CNN
F 3 "~" H 2100 5450 50  0001 C CNN
	1    2100 5450
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0180
U 1 1 6071E102
P 2100 5700
F 0 "#PWR0180" H 2100 5450 50  0001 C CNN
F 1 "GND" H 2105 5527 50  0000 C CNN
F 2 "" H 2100 5700 50  0001 C CNN
F 3 "" H 2100 5700 50  0001 C CNN
	1    2100 5700
	1    0    0    -1  
$EndComp
Wire Wire Line
	2100 5600 2100 5700
Wire Wire Line
	2100 5300 2100 5050
Connection ~ 2100 5050
Wire Wire Line
	2100 5050 2050 5050
$EndSCHEMATC
