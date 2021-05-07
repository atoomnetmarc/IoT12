EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Converter-ACDC-extra:MPM-90-24 PS1
U 1 1 6038DC20
P 5500 2400
F 0 "PS1" H 5500 2767 50  0000 C CNN
F 1 "MPM-90-24" H 5500 2676 50  0000 C CNN
F 2 "Converter-ACDC-extra:Converter_ACDC_MeanWell_MPM-90-xx_THT" H 5500 2000 50  0001 C CNN
F 3 "" H 5500 2400 50  0001 C CNN
	1    5500 2400
	1    0    0    -1  
$EndComp
$Comp
L Connector:Screw_Terminal_01x03 J1
U 1 1 6038EB43
P 4300 2450
F 0 "J1" H 4218 2125 50  0000 C CNN
F 1 "Screw_Terminal_01x03" H 4218 2216 50  0000 C CNN
F 2 "TerminalBlock:TerminalBlock_bornier-3_P5.08mm" H 4300 2450 50  0001 C CNN
F 3 "~" H 4300 2450 50  0001 C CNN
	1    4300 2450
	-1   0    0    1   
$EndComp
$Comp
L Connector:Screw_Terminal_01x03 J2
U 1 1 60390401
P 6750 2450
F 0 "J2" H 6668 2125 50  0000 C CNN
F 1 "Screw_Terminal_01x03" H 6668 2216 50  0000 C CNN
F 2 "TerminalBlock:TerminalBlock_bornier-3_P5.08mm" H 6750 2450 50  0001 C CNN
F 3 "~" H 6750 2450 50  0001 C CNN
	1    6750 2450
	1    0    0    1   
$EndComp
Wire Wire Line
	4500 2350 4850 2350
Wire Wire Line
	4850 2350 4850 2300
Wire Wire Line
	4850 2300 5100 2300
Wire Wire Line
	4500 2450 4850 2450
Wire Wire Line
	4850 2450 4850 2500
Wire Wire Line
	4850 2500 5100 2500
Wire Wire Line
	5900 2300 6200 2300
Wire Wire Line
	6200 2300 6200 2350
Wire Wire Line
	6200 2350 6550 2350
Wire Wire Line
	5900 2500 6200 2500
Wire Wire Line
	6200 2500 6200 2450
Wire Wire Line
	6200 2450 6550 2450
Wire Wire Line
	6550 2550 6200 2550
Wire Wire Line
	6200 2550 6200 2700
Wire Wire Line
	6200 2700 5500 2700
Wire Wire Line
	4850 2700 4850 2550
Wire Wire Line
	4850 2550 4500 2550
$Comp
L power:Earth #PWR01
U 1 1 60392135
P 5500 2850
F 0 "#PWR01" H 5500 2600 50  0001 C CNN
F 1 "Earth" H 5500 2700 50  0001 C CNN
F 2 "" H 5500 2850 50  0001 C CNN
F 3 "~" H 5500 2850 50  0001 C CNN
	1    5500 2850
	1    0    0    -1  
$EndComp
Wire Wire Line
	5500 2850 5500 2700
Connection ~ 5500 2700
Wire Wire Line
	5500 2700 4850 2700
$Comp
L Mechanical:MountingHole_Pad H1
U 1 1 60393027
P 5450 5650
F 0 "H1" V 5404 5800 50  0000 L CNN
F 1 "MountingHole" V 5495 5800 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3_Pad_Via" H 5450 5650 50  0001 C CNN
F 3 "~" H 5450 5650 50  0001 C CNN
	1    5450 5650
	0    1    1    0   
$EndComp
Text Label 4600 2350 0    50   ~ 0
L
Text Label 4600 2450 0    50   ~ 0
N
Text Label 6250 2350 0    50   ~ 0
Vout+
Text Label 6250 2450 0    50   ~ 0
Vout-
$Comp
L Mechanical:MountingHole_Pad H2
U 1 1 6039407F
P 5450 5850
F 0 "H2" V 5404 6000 50  0000 L CNN
F 1 "MountingHole" V 5495 6000 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3_Pad_Via" H 5450 5850 50  0001 C CNN
F 3 "~" H 5450 5850 50  0001 C CNN
	1    5450 5850
	0    1    1    0   
$EndComp
$Comp
L Mechanical:MountingHole_Pad H3
U 1 1 6039426C
P 5450 6050
F 0 "H3" V 5404 6200 50  0000 L CNN
F 1 "MountingHole" V 5495 6200 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3_Pad_Via" H 5450 6050 50  0001 C CNN
F 3 "~" H 5450 6050 50  0001 C CNN
	1    5450 6050
	0    1    1    0   
$EndComp
$Comp
L Mechanical:MountingHole_Pad H4
U 1 1 60394474
P 5450 6250
F 0 "H4" V 5404 6400 50  0000 L CNN
F 1 "MountingHole" V 5495 6400 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3_Pad_Via" H 5450 6250 50  0001 C CNN
F 3 "~" H 5450 6250 50  0001 C CNN
	1    5450 6250
	0    1    1    0   
$EndComp
$Comp
L Converter-ACDC-extra:MPM-30-15 PS2
U 1 1 603A0519
P 5500 3850
F 0 "PS2" H 5500 4217 50  0000 C CNN
F 1 "MPM-30-15*" H 5500 4126 50  0000 C CNN
F 2 "Converter-ACDC-extra:Converter_ACDC_MeanWell_MPM-30-xx_THT" H 5500 3450 50  0001 C CNN
F 3 "" H 5500 3850 50  0001 C CNN
	1    5500 3850
	1    0    0    -1  
$EndComp
Wire Wire Line
	5100 3750 4650 3750
Wire Wire Line
	5100 3950 4650 3950
Wire Wire Line
	5900 3750 6400 3750
Wire Wire Line
	5900 3950 6400 3950
Text Label 4650 3750 0    50   ~ 0
L
Text Label 4650 3950 0    50   ~ 0
N
Text Label 8000 3000 3    50   ~ 0
Vout+
Text Label 6150 3950 0    50   ~ 0
Vout-
Wire Notes Line
	4200 3150 6850 3150
Wire Notes Line
	6850 3150 6850 5350
Wire Notes Line
	6850 5350 4200 5350
Wire Notes Line
	4200 5350 4200 3150
Text Notes 4250 3300 0    50   ~ 0
Lower power alternatives
$Comp
L Converter-ACDC-extra:MPM-20-12 PS3
U 1 1 603A726D
P 5500 4650
F 0 "PS3" H 5500 5017 50  0000 C CNN
F 1 "MPM-20-12*" H 5500 4926 50  0000 C CNN
F 2 "Converter-ACDC-extra:Converter_ACDC_MeanWell_MPM-20-xx_THT" H 5500 4250 50  0001 C CNN
F 3 "" H 5500 4650 50  0001 C CNN
	1    5500 4650
	1    0    0    -1  
$EndComp
Wire Wire Line
	5100 4550 4650 4550
Wire Wire Line
	5100 4750 4650 4750
Wire Wire Line
	5900 4550 6400 4550
Wire Wire Line
	5900 4750 6400 4750
Text Label 4650 4550 0    50   ~ 0
L
Text Label 4650 4750 0    50   ~ 0
N
Text Label 6150 4550 0    50   ~ 0
Vout+
Text Label 6150 4750 0    50   ~ 0
Vout-
NoConn ~ 5350 5650
NoConn ~ 5350 5850
NoConn ~ 5350 6050
NoConn ~ 5350 6250
$Comp
L Device:LED D2
U 1 1 603B83D7
P 8000 4100
F 0 "D2" V 8039 3982 50  0000 R CNN
F 1 "Green" V 7948 3982 50  0000 R CNN
F 2 "LED_SMD:LED_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 8000 4100 50  0001 C CNN
F 3 "~" H 8000 4100 50  0001 C CNN
	1    8000 4100
	0    -1   -1   0   
$EndComp
Wire Wire Line
	8000 3950 8000 3700
Wire Wire Line
	8000 4250 8000 4700
Text Label 8000 4700 1    50   ~ 0
Vout-
$Comp
L Device:R R2
U 1 1 603B9D14
P 8000 3550
F 0 "R2" H 8070 3596 50  0000 L CNN
F 1 "4K7" H 8070 3505 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 7930 3550 50  0001 C CNN
F 3 "~" H 8000 3550 50  0001 C CNN
	1    8000 3550
	1    0    0    -1  
$EndComp
Wire Wire Line
	8000 3400 8000 3000
Text Label 6400 3750 2    50   ~ 0
Vout+
$EndSCHEMATC
