EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 2 5
Title "IoT12-hardware input and sensors"
Date "2021-09-26"
Rev "V2.1"
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L power:GND #PWR0119
U 1 1 5FF2A4BA
P 2150 7400
F 0 "#PWR0119" H 2150 7150 50  0001 C CNN
F 1 "GND" H 2155 7227 50  0000 C CNN
F 2 "" H 2150 7400 50  0001 C CNN
F 3 "" H 2150 7400 50  0001 C CNN
	1    2150 7400
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0118
U 1 1 6033938D
P 1350 7400
F 0 "#PWR0118" H 1350 7150 50  0001 C CNN
F 1 "GND" H 1355 7227 50  0000 C CNN
F 2 "" H 1350 7400 50  0001 C CNN
F 3 "" H 1350 7400 50  0001 C CNN
	1    1350 7400
	1    0    0    -1  
$EndComp
Wire Wire Line
	1450 6650 1350 6650
Wire Wire Line
	1350 6650 1350 7400
Wire Wire Line
	2050 6750 2150 6750
Wire Wire Line
	2150 6750 2150 7400
Text GLabel 900  6550 0    50   Output ~ 0
ROT_A
Text GLabel 900  6750 0    50   Output ~ 0
ROT_B
Text GLabel 2450 6550 2    50   Output ~ 0
ROT_SW
Wire Notes Line
	550  5750 550  7700
Wire Notes Line
	550  7700 2850 7700
Wire Notes Line
	2850 7700 2850 5750
Wire Notes Line
	2850 5750 550  5750
Text Notes 600  5950 0    118  ~ 0
User input
Wire Notes Line
	550  550  550  3050
Wire Notes Line
	550  3050 6250 3050
Wire Notes Line
	6250 3050 6250 550 
Wire Notes Line
	6250 550  550  550 
Text Notes 600  750  0    118  ~ 0
Heater temperature sensor
Wire Notes Line
	6350 3050 6350 550 
Wire Notes Line
	6350 550  11100 550 
Wire Notes Line
	11100 3050 6350 3050
Wire Notes Line
	11100 550  11100 3050
Text Notes 6400 750  0    118  ~ 0
Heater current sensor
Text GLabel 10350 1800 2    50   Output ~ 0
IRON_CURRENT
$Comp
L Device:Rotary_Encoder_Switch SW1
U 1 1 5FF2935D
P 1750 6650
F 0 "SW1" H 1750 7017 50  0000 C CNN
F 1 "Rotary encoder EC11 Plum 15mm" H 1750 6926 50  0000 C CNN
F 2 "Rotary_Encoder-fixed:RotaryEncoder_Alps_EC11E-Switch_Vertical_H20mm_CircularMountingHoles" H 1600 6810 50  0001 C CNN
F 3 "~" H 1750 6910 50  0001 C CNN
	1    1750 6650
	1    0    0    -1  
$EndComp
Wire Notes Line
	4350 5650 550  5650
Wire Notes Line
	550  3150 4350 3150
Wire Notes Line
	4350 3150 4350 5650
Text Notes 3000 6000 0    118  ~ 0
Handle NTC
$Comp
L Connector_Generic:Conn_01x01 J2
U 1 1 600FEFC0
P 3200 7200
F 0 "J2" H 3118 7067 50  0001 C CNN
F 1 "~" H 3118 7066 50  0001 C CNN
F 2 "TestPoint:TestPoint_THTPad_D2.0mm_Drill1.0mm" H 3200 7200 50  0001 C CNN
F 3 "~" H 3200 7200 50  0001 C CNN
	1    3200 7200
	-1   0    0    1   
$EndComp
Wire Wire Line
	3400 7200 3450 7200
Wire Wire Line
	3450 6800 3450 7200
Connection ~ 3450 7200
$Comp
L power:+3.3V #PWR0117
U 1 1 60133809
P 3450 6400
F 0 "#PWR0117" H 3450 6250 50  0001 C CNN
F 1 "+3.3V" H 3465 6573 50  0000 C CNN
F 2 "" H 3450 6400 50  0001 C CNN
F 3 "" H 3450 6400 50  0001 C CNN
	1    3450 6400
	1    0    0    -1  
$EndComp
Wire Wire Line
	3450 6500 3450 6400
Text GLabel 4000 7200 2    50   Output ~ 0
NTC
Wire Wire Line
	3800 7200 4000 7200
Text Notes 3150 7150 0    50   ~ 0
NTC
$Comp
L Connector_Generic:Conn_01x01 J1
U 1 1 6027A2F4
P 1300 4500
F 0 "J1" H 1218 4367 50  0001 C CNN
F 1 "~" H 1218 4366 50  0001 C CNN
F 2 "TestPoint:TestPoint_THTPad_D2.0mm_Drill1.0mm" H 1300 4500 50  0001 C CNN
F 3 "~" H 1300 4500 50  0001 C CNN
	1    1300 4500
	-1   0    0    1   
$EndComp
Text Notes 1050 4450 0    50   ~ 0
Shake
$Comp
L Device:R R80
U 1 1 6028B458
P 2550 4100
F 0 "R80" H 2620 4146 50  0000 L CNN
F 1 "1M" H 2620 4055 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 2480 4100 50  0001 C CNN
F 3 "~" H 2550 4100 50  0001 C CNN
	1    2550 4100
	1    0    0    -1  
$EndComp
Wire Wire Line
	2150 4500 2550 4500
Wire Wire Line
	2550 4500 2550 4250
Wire Wire Line
	2550 3950 2550 3650
$Comp
L Device:C C10
U 1 1 603393A0
P 2550 4850
F 0 "C10" H 2665 4896 50  0000 L CNN
F 1 "100pF" H 2665 4805 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder" H 2588 4700 50  0001 C CNN
F 3 "~" H 2550 4850 50  0001 C CNN
	1    2550 4850
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0113
U 1 1 603393A1
P 2550 5350
F 0 "#PWR0113" H 2550 5100 50  0001 C CNN
F 1 "GND" H 2555 5177 50  0000 C CNN
F 2 "" H 2550 5350 50  0001 C CNN
F 3 "" H 2550 5350 50  0001 C CNN
	1    2550 5350
	1    0    0    -1  
$EndComp
Wire Wire Line
	2550 4700 2550 4500
Connection ~ 2550 4500
Wire Wire Line
	2550 5000 2550 5350
Wire Notes Line
	2950 5750 2950 7700
Wire Notes Line
	2950 7700 4350 7700
Wire Notes Line
	4350 7700 4350 5750
Wire Notes Line
	4350 5750 2950 5750
Text Notes 600  3350 0    118  ~ 0
Tilt switch sensor as shake sensor
$Comp
L power:+3.3V #PWR0109
U 1 1 6028C38E
P 2550 3650
F 0 "#PWR0109" H 2550 3500 50  0001 C CNN
F 1 "+3.3V" H 2565 3823 50  0000 C CNN
F 2 "" H 2550 3650 50  0001 C CNN
F 3 "" H 2550 3650 50  0001 C CNN
	1    2550 3650
	1    0    0    -1  
$EndComp
$Comp
L Device:R R81
U 1 1 603393B1
P 2000 4500
F 0 "R81" V 1793 4500 50  0000 C CNN
F 1 "1M" V 1884 4500 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 1930 4500 50  0001 C CNN
F 3 "~" H 2000 4500 50  0001 C CNN
	1    2000 4500
	0    1    1    0   
$EndComp
Wire Wire Line
	3450 7200 3500 7200
$Comp
L Device:R R62
U 1 1 603393A3
P 7750 1450
F 0 "R62" V 7543 1450 50  0000 C CNN
F 1 "10k 0.1%" V 7634 1450 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 7680 1450 50  0001 C CNN
F 3 "~" H 7750 1450 50  0001 C CNN
	1    7750 1450
	0    1    1    0   
$EndComp
$Comp
L Amplifier_Instrumentation-extra:INA826 U4
U 1 1 603393A4
P 9300 1800
F 0 "U4" H 9450 1950 50  0000 L CNN
F 1 "INA826AID" H 9450 1650 50  0000 L CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 9300 1800 50  0001 C CNN
F 3 "http://www.ti.com/product/INA826/" H 9300 1800 50  0001 C CNN
	1    9300 1800
	1    0    0    -1  
$EndComp
$Comp
L Device:R R30
U 1 1 603393A5
P 8850 1800
F 0 "R30" H 8920 1754 50  0000 L CNN
F 1 "1k 1%" H 8920 1845 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 8780 1800 50  0001 C CNN
F 3 "~" H 8850 1800 50  0001 C CNN
	1    8850 1800
	-1   0    0    1   
$EndComp
Wire Wire Line
	8950 1900 9000 1900
Wire Wire Line
	8950 1700 9000 1700
Wire Wire Line
	8950 1700 8950 1650
Wire Wire Line
	8950 1650 8850 1650
Wire Wire Line
	8950 1900 8950 1950
Wire Wire Line
	8950 1950 8850 1950
Text Notes 8750 2050 0    50   ~ 0
50.4G
Wire Wire Line
	9400 2100 9400 2250
Wire Wire Line
	9400 2250 9450 2250
Text GLabel 9450 2250 2    50   Input ~ 0
NEAR_ADC_GND
$Comp
L power:GND #PWR0108
U 1 1 603393A6
P 9300 2750
F 0 "#PWR0108" H 9300 2500 50  0001 C CNN
F 1 "GND" H 9305 2577 50  0000 C CNN
F 2 "" H 9300 2750 50  0001 C CNN
F 3 "" H 9300 2750 50  0001 C CNN
	1    9300 2750
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR0102
U 1 1 603393A7
P 9300 850
F 0 "#PWR0102" H 9300 700 50  0001 C CNN
F 1 "+3.3V" H 9315 1023 50  0000 C CNN
F 2 "" H 9300 850 50  0001 C CNN
F 3 "" H 9300 850 50  0001 C CNN
	1    9300 850 
	1    0    0    -1  
$EndComp
Wire Wire Line
	9300 2100 9300 2750
$Comp
L Device:R R11
U 1 1 603393A8
P 9300 1050
F 0 "R11" V 9093 1050 50  0000 C CNN
F 1 "100R" V 9184 1050 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 9230 1050 50  0001 C CNN
F 3 "~" H 9300 1050 50  0001 C CNN
	1    9300 1050
	-1   0    0    1   
$EndComp
Wire Wire Line
	9300 900  9300 850 
Wire Wire Line
	9300 1200 9300 1500
$Comp
L Device:C C31
U 1 1 603393A9
P 9700 1350
F 0 "C31" H 9815 1396 50  0000 L CNN
F 1 "100nF" H 9815 1305 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder" H 9738 1200 50  0001 C CNN
F 3 "~" H 9700 1350 50  0001 C CNN
	1    9700 1350
	1    0    0    -1  
$EndComp
Wire Wire Line
	9700 1200 9300 1200
Connection ~ 9300 1200
$Comp
L power:GND #PWR0104
U 1 1 603393AA
P 9700 1550
F 0 "#PWR0104" H 9700 1300 50  0001 C CNN
F 1 "GND" H 9788 1513 50  0000 L CNN
F 2 "" H 9700 1550 50  0001 C CNN
F 3 "" H 9700 1550 50  0001 C CNN
	1    9700 1550
	1    0    0    -1  
$EndComp
$Comp
L Device:C C21
U 1 1 6049FE3D
P 8050 1800
F 0 "C21" H 8200 1650 50  0000 R CNN
F 1 "1nF" H 8250 1950 50  0000 R CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder" H 8088 1650 50  0001 C CNN
F 3 "~" H 8050 1800 50  0001 C CNN
	1    8050 1800
	1    0    0    -1  
$EndComp
$Comp
L Device:R R63
U 1 1 6049FE47
P 7750 2150
F 0 "R63" V 7543 2150 50  0000 C CNN
F 1 "10k 0.1%" V 7634 2150 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 7680 2150 50  0001 C CNN
F 3 "~" H 7750 2150 50  0001 C CNN
	1    7750 2150
	0    1    1    0   
$EndComp
Wire Wire Line
	9000 2000 9000 2150
Wire Wire Line
	9000 1600 9000 1450
Wire Wire Line
	8050 1650 8050 1450
Connection ~ 8050 1450
Wire Wire Line
	8050 1450 7900 1450
Wire Wire Line
	8050 1950 8050 2150
Connection ~ 8050 2150
Wire Wire Line
	8050 2150 7900 2150
Wire Wire Line
	7300 2150 7600 2150
Wire Wire Line
	9700 1550 9700 1500
Wire Wire Line
	1600 2150 1700 2150
Wire Wire Line
	2200 2150 2000 2150
Connection ~ 2200 2150
Wire Wire Line
	2200 1950 2200 2150
Wire Wire Line
	2200 1450 2000 1450
Connection ~ 2200 1450
Wire Wire Line
	2200 1650 2200 1450
Wire Wire Line
	1600 1450 1700 1450
$Comp
L Device:R R61
U 1 1 60339398
P 1850 2150
F 0 "R61" V 1643 2150 50  0000 C CNN
F 1 "10k 0.1%" V 1734 2150 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 1780 2150 50  0001 C CNN
F 3 "~" H 1850 2150 50  0001 C CNN
	1    1850 2150
	0    1    1    0   
$EndComp
$Comp
L Device:R R60
U 1 1 603393A2
P 1850 1450
F 0 "R60" V 1643 1450 50  0000 C CNN
F 1 "10k 0.1%" V 1734 1450 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 1780 1450 50  0001 C CNN
F 3 "~" H 1850 1450 50  0001 C CNN
	1    1850 1450
	0    1    1    0   
$EndComp
Text GLabel 1600 1450 0    50   Input ~ 0
IRON_+
Text GLabel 7300 2150 0    50   Input ~ 0
NEAR_RSENSE_-
Wire Wire Line
	7300 1450 7600 1450
Text GLabel 7300 1450 0    50   Input ~ 0
NEAR_RSENSE_+
$Comp
L power:PWR_FLAG #FLG0102
U 1 1 606E8615
P 9700 850
F 0 "#FLG0102" H 9700 925 50  0001 C CNN
F 1 "PWR_FLAG" H 9700 1023 50  0000 C CNN
F 2 "" H 9700 850 50  0001 C CNN
F 3 "~" H 9700 850 50  0001 C CNN
	1    9700 850 
	1    0    0    -1  
$EndComp
Wire Wire Line
	9700 850  9700 1200
Connection ~ 9700 1200
Wire Notes Line
	1650 2250 2050 2250
Wire Notes Line
	2050 2250 2050 1000
Wire Notes Line
	2050 1000 1650 1000
Wire Notes Line
	1650 1000 1650 2250
Text Notes 1700 1100 0    50   ~ 0
Matched*
Text Notes 7600 1100 0    50   ~ 0
Matched*
Wire Notes Line
	7950 2250 7550 2250
Wire Notes Line
	7550 2250 7550 1000
Wire Notes Line
	7550 1000 7950 1000
Wire Notes Line
	7950 1000 7950 2250
Text Notes 6900 1900 0    50   ~ 0
0.005V@1A\n0.015V@3A\n0.025V@5A
Wire Notes Line
	550  5650 550  3150
Wire Wire Line
	4650 850  4650 1200
$Comp
L power:PWR_FLAG #FLG0101
U 1 1 607033B8
P 4650 850
F 0 "#FLG0101" H 4650 925 50  0001 C CNN
F 1 "PWR_FLAG" H 4650 1023 50  0000 C CNN
F 2 "" H 4650 850 50  0001 C CNN
F 3 "~" H 4650 850 50  0001 C CNN
	1    4650 850 
	1    0    0    -1  
$EndComp
Wire Wire Line
	3950 2150 3350 2150
Wire Wire Line
	3950 1450 2800 1450
Wire Wire Line
	4650 1550 4650 1500
Wire Wire Line
	3950 1600 3950 1450
Wire Wire Line
	3950 2000 3950 2150
$Comp
L power:GND #PWR0103
U 1 1 60339396
P 4650 1550
F 0 "#PWR0103" H 4650 1300 50  0001 C CNN
F 1 "GND" H 4738 1513 50  0000 L CNN
F 2 "" H 4650 1550 50  0001 C CNN
F 3 "" H 4650 1550 50  0001 C CNN
	1    4650 1550
	1    0    0    -1  
$EndComp
Connection ~ 4250 1200
Wire Wire Line
	4650 1200 4250 1200
Wire Wire Line
	4250 1200 4250 1500
Wire Wire Line
	4250 900  4250 850 
$Comp
L Device:R R10
U 1 1 60339390
P 4250 1050
F 0 "R10" V 4043 1050 50  0000 C CNN
F 1 "100R" V 4134 1050 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 4180 1050 50  0001 C CNN
F 3 "~" H 4250 1050 50  0001 C CNN
	1    4250 1050
	-1   0    0    1   
$EndComp
Text GLabel 5500 1800 2    50   Output ~ 0
TEMP
Wire Wire Line
	4250 2100 4250 2650
$Comp
L power:+3.3V #PWR0101
U 1 1 6018FE62
P 4250 850
F 0 "#PWR0101" H 4250 700 50  0001 C CNN
F 1 "+3.3V" H 4265 1023 50  0000 C CNN
F 2 "" H 4250 850 50  0001 C CNN
F 3 "" H 4250 850 50  0001 C CNN
	1    4250 850 
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0107
U 1 1 6018F510
P 4250 2650
F 0 "#PWR0107" H 4250 2400 50  0001 C CNN
F 1 "GND" H 4255 2477 50  0000 C CNN
F 2 "" H 4250 2650 50  0001 C CNN
F 3 "" H 4250 2650 50  0001 C CNN
	1    4250 2650
	1    0    0    -1  
$EndComp
Text GLabel 4400 2250 2    50   Input ~ 0
NEAR_ADC_GND
Wire Wire Line
	4350 2250 4400 2250
Wire Wire Line
	4350 2100 4350 2250
Text Notes 3450 2000 0    50   ~ 0
106.1G
Wire Wire Line
	3900 1950 3800 1950
Wire Wire Line
	3900 1900 3900 1950
Wire Wire Line
	3900 1650 3800 1650
Wire Wire Line
	3900 1700 3900 1650
Wire Wire Line
	3900 1700 3950 1700
Wire Wire Line
	3900 1900 3950 1900
$Comp
L Device:R R21
U 1 1 6005AD80
P 3800 1800
F 0 "R21" H 3870 1754 50  0000 L CNN
F 1 "470R 1%" H 3870 1845 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 3730 1800 50  0001 C CNN
F 3 "~" H 3800 1800 50  0001 C CNN
	1    3800 1800
	-1   0    0    1   
$EndComp
$Comp
L Amplifier_Instrumentation-extra:INA826 U3
U 1 1 60057F60
P 4250 1800
F 0 "U3" H 4400 1950 50  0000 L CNN
F 1 "INA826AID" H 4400 1650 50  0000 L CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 4250 1800 50  0001 C CNN
F 3 "http://www.ti.com/product/INA826/" H 4250 1800 50  0001 C CNN
	1    4250 1800
	1    0    0    -1  
$EndComp
$Comp
L Device:C C20
U 1 1 60152986
P 2200 1800
F 0 "C20" H 2400 1650 50  0000 R CNN
F 1 "1nF" H 2400 1950 50  0000 R CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder" H 2238 1650 50  0001 C CNN
F 3 "~" H 2200 1800 50  0001 C CNN
	1    2200 1800
	1    0    0    -1  
$EndComp
Text Notes 950  1850 0    50   ~ 0
8.657mV@500°C\n(Tamb = 0°C)
Text Notes 5200 1700 0    50   ~ 0
0.92V@500°C
Text Notes 3750 7350 0    50   ~ 0
1.65V@25°C
$Comp
L Device:R R51
U 1 1 60339393
P 3650 7200
F 0 "R51" V 3443 7200 50  0000 C CNN
F 1 "4k7" V 3534 7200 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 3580 7200 50  0001 C CNN
F 3 "~" H 3650 7200 50  0001 C CNN
	1    3650 7200
	0    1    1    0   
$EndComp
$Comp
L Device:R R64
U 1 1 60339392
P 3450 6650
F 0 "R64" H 3520 6696 50  0000 L CNN
F 1 "10k 0.1%" H 3520 6605 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 3380 6650 50  0001 C CNN
F 3 "~" H 3450 6650 50  0001 C CNN
	1    3450 6650
	1    0    0    -1  
$EndComp
Text GLabel 1600 2150 0    50   Input ~ 0
NEAR_RSENSE_+
Wire Wire Line
	2550 4500 3300 4500
Wire Wire Line
	1500 4500 1850 4500
Text Notes 900  2850 0    50   ~ 0
Matched*: value does not matter that much, \nbut resistors must be matched to equal value.\nYou may use oridinary 5% resitors and \ncherry-pick a perfectly equal pair.
Text Notes 5150 1550 0    50   ~ 0
Keep output \nunder 1V at \nfull scale input.
Text Notes 10250 1550 0    50   ~ 0
Keep output \nunder 1V at \nfull scale input.
Text Notes 10350 1700 0    50   ~ 0
0.756V@3A
$Comp
L Diode:BAV99 D2
U 1 1 602F5ECE
P 3200 2250
F 0 "D2" V 3250 2050 50  0000 L CNN
F 1 "BAV99" V 3200 2300 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 3200 1750 50  0001 C CNN
F 3 "https://assets.nexperia.com/documents/data-sheet/BAV99_SER.pdf" H 3200 2250 50  0001 C CNN
	1    3200 2250
	0    1    1    0   
$EndComp
Wire Wire Line
	8050 1450 9000 1450
Wire Wire Line
	8050 2150 9000 2150
$Comp
L Diode:BAV99 D1
U 1 1 6033052D
P 2800 1800
F 0 "D1" V 2850 1600 50  0000 L CNN
F 1 "BAV99" V 2650 1500 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 2800 1300 50  0001 C CNN
F 3 "https://assets.nexperia.com/documents/data-sheet/BAV99_SER.pdf" H 2800 1800 50  0001 C CNN
	1    2800 1800
	0    -1   1    0   
$EndComp
Wire Wire Line
	2800 1500 2800 1450
Connection ~ 2800 1450
Wire Wire Line
	2800 1450 2200 1450
Wire Wire Line
	3200 1950 3350 1950
Wire Wire Line
	3350 1950 3350 2150
Connection ~ 3350 2150
Wire Wire Line
	3350 2150 2200 2150
Wire Wire Line
	3000 2250 3000 2550
Wire Wire Line
	3000 2550 3200 2550
Wire Wire Line
	3000 1800 3000 2100
Wire Wire Line
	3000 2100 2800 2100
$Comp
L power:GND #PWR0106
U 1 1 603393AD
P 3200 2650
F 0 "#PWR0106" H 3200 2400 50  0001 C CNN
F 1 "GND" H 3205 2477 50  0000 C CNN
F 2 "" H 3200 2650 50  0001 C CNN
F 3 "" H 3200 2650 50  0001 C CNN
	1    3200 2650
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0105
U 1 1 603393AE
P 2800 2650
F 0 "#PWR0105" H 2800 2400 50  0001 C CNN
F 1 "GND" H 2805 2477 50  0000 C CNN
F 2 "" H 2800 2650 50  0001 C CNN
F 3 "" H 2800 2650 50  0001 C CNN
	1    2800 2650
	1    0    0    -1  
$EndComp
Wire Wire Line
	2800 2650 2800 2100
Connection ~ 2800 2100
Wire Wire Line
	3200 2650 3200 2550
Connection ~ 3200 2550
Connection ~ 4650 1200
$Comp
L Device:C C30
U 1 1 60339394
P 4650 1350
F 0 "C30" H 4765 1396 50  0000 L CNN
F 1 "100nF" H 4765 1305 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder" H 4688 1200 50  0001 C CNN
F 3 "~" H 4650 1350 50  0001 C CNN
	1    4650 1350
	1    0    0    -1  
$EndComp
Text GLabel 3300 4500 2    50   Output ~ 0
AIN_COMP
Wire Wire Line
	900  6750 1450 6750
Wire Wire Line
	900  6550 1450 6550
Wire Wire Line
	2050 6550 2450 6550
Wire Wire Line
	4650 1800 5500 1800
Wire Wire Line
	9700 1800 10350 1800
Text GLabel 7550 4750 0    50   Input ~ 0
TEMP
Text GLabel 7550 4950 0    50   Input ~ 0
IRON_CURRENT
Text GLabel 7550 5050 0    50   Input ~ 0
VIN
Text GLabel 7550 4850 0    50   Input ~ 0
NTC
$Comp
L Analog_ADC:ADS1115IDGS U2
U 1 1 601A48E7
P 8600 4850
F 0 "U2" H 8600 5531 50  0000 C CNN
F 1 "ADS1115IDGS" H 8600 5440 50  0000 C CNN
F 2 "Package_SO:TSSOP-10_3x3mm_P0.5mm" H 8600 4350 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/ads1113.pdf" H 8550 3950 50  0001 C CNN
	1    8600 4850
	1    0    0    -1  
$EndComp
Wire Notes Line
	6350 3150 11100 3150
Wire Notes Line
	11100 6450 6350 6450
Wire Notes Line
	6350 6450 6350 3150
Text Notes 6400 3350 0    118  ~ 0
External 16b I2C ADC
$Comp
L power:GND #PWR0115
U 1 1 6026AD10
P 8600 6150
F 0 "#PWR0115" H 8600 5900 50  0001 C CNN
F 1 "GND" H 8605 5977 50  0000 C CNN
F 2 "" H 8600 6150 50  0001 C CNN
F 3 "" H 8600 6150 50  0001 C CNN
	1    8600 6150
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR0111
U 1 1 6026B468
P 8600 3650
F 0 "#PWR0111" H 8600 3500 50  0001 C CNN
F 1 "+3.3V" H 8615 3823 50  0000 C CNN
F 2 "" H 8600 3650 50  0001 C CNN
F 3 "" H 8600 3650 50  0001 C CNN
	1    8600 3650
	1    0    0    -1  
$EndComp
Wire Wire Line
	8600 3650 8600 4350
Wire Wire Line
	8600 5250 8600 5950
Text GLabel 10400 4950 2    50   BiDi ~ 0
HEAT_SDA
Text GLabel 10400 4850 2    50   Input ~ 0
HEAT_SCL
Wire Wire Line
	9000 5050 9200 5050
Wire Wire Line
	9200 5050 9200 6150
$Comp
L power:GND #PWR0116
U 1 1 60379B2A
P 9200 6150
F 0 "#PWR0116" H 9200 5900 50  0001 C CNN
F 1 "GND" H 9205 5977 50  0000 C CNN
F 2 "" H 9200 6150 50  0001 C CNN
F 3 "" H 9200 6150 50  0001 C CNN
	1    9200 6150
	1    0    0    -1  
$EndComp
Text Notes 8650 5250 0    50   ~ 0
I2C: 0x48
Text GLabel 7850 5950 0    50   Input ~ 0
NEAR_ADC_GND
$Comp
L Device:Net-Tie_2 NT1
U 1 1 6037B2CF
P 8400 5950
F 0 "NT1" H 8400 6039 50  0000 C CNN
F 1 "~" H 8400 6040 50  0000 C CNN
F 2 "NetTie-extra:NetTie-2_SMD_Pad0.25mm" H 8400 5950 50  0001 C CNN
F 3 "~" H 8400 5950 50  0001 C CNN
	1    8400 5950
	1    0    0    -1  
$EndComp
Wire Wire Line
	7850 5950 8300 5950
Wire Wire Line
	8500 5950 8600 5950
Connection ~ 8600 5950
Wire Wire Line
	8600 5950 8600 6150
$Comp
L power:GND #PWR0114
U 1 1 603AF428
P 6600 6150
F 0 "#PWR0114" H 6600 5900 50  0001 C CNN
F 1 "GND" H 6605 5977 50  0000 C CNN
F 2 "" H 6600 6150 50  0001 C CNN
F 3 "" H 6600 6150 50  0001 C CNN
	1    6600 6150
	1    0    0    -1  
$EndComp
Wire Wire Line
	6600 3650 6600 5250
Wire Wire Line
	6600 5550 6600 6150
$Comp
L Device:C C32
U 1 1 603B0236
P 6600 5400
F 0 "C32" H 6715 5446 50  0000 L CNN
F 1 "100nF" H 6715 5355 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder" H 6638 5250 50  0001 C CNN
F 3 "~" H 6600 5400 50  0001 C CNN
	1    6600 5400
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR0110
U 1 1 603E71AD
P 6600 3650
F 0 "#PWR0110" H 6600 3500 50  0001 C CNN
F 1 "+3.3V" H 6615 3823 50  0000 C CNN
F 2 "" H 6600 3650 50  0001 C CNN
F 3 "" H 6600 3650 50  0001 C CNN
	1    6600 3650
	1    0    0    -1  
$EndComp
Wire Wire Line
	7550 4750 8200 4750
Wire Wire Line
	8200 4850 7550 4850
Wire Wire Line
	7550 4950 8200 4950
Wire Wire Line
	8200 5050 7550 5050
$Comp
L Device:R R50
U 1 1 605429D4
P 10100 4150
F 0 "R50" H 10030 4104 50  0000 R CNN
F 1 "4k7" H 10030 4195 50  0000 R CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 10030 4150 50  0001 C CNN
F 3 "~" H 10100 4150 50  0001 C CNN
	1    10100 4150
	-1   0    0    1   
$EndComp
Text GLabel 10400 4650 2    50   Output ~ 0
ADC_RDY
Wire Wire Line
	9000 4650 10100 4650
Wire Wire Line
	10100 4300 10100 4650
Connection ~ 10100 4650
Wire Wire Line
	10100 4650 10400 4650
$Comp
L power:+3.3V #PWR0112
U 1 1 605CE7EF
P 10100 3650
F 0 "#PWR0112" H 10100 3500 50  0001 C CNN
F 1 "+3.3V" H 10115 3823 50  0000 C CNN
F 2 "" H 10100 3650 50  0001 C CNN
F 3 "" H 10100 3650 50  0001 C CNN
	1    10100 3650
	1    0    0    -1  
$EndComp
Wire Notes Line
	11100 3150 11100 6450
Wire Wire Line
	9000 4950 10400 4950
Wire Wire Line
	9000 4850 10400 4850
Wire Wire Line
	10100 3650 10100 4000
$EndSCHEMATC
