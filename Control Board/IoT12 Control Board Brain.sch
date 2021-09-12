EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 4 5
Title "IoT12-hardware brain"
Date "2021-09-12"
Rev "V2.1"
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L power:GND #PWR0153
U 1 1 5FF0E00E
P 2650 4850
F 0 "#PWR0153" H 2650 4600 50  0001 C CNN
F 1 "GND" H 2655 4677 50  0000 C CNN
F 2 "" H 2650 4850 50  0001 C CNN
F 3 "" H 2650 4850 50  0001 C CNN
	1    2650 4850
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR0132
U 1 1 5FF0EBE1
P 2550 1050
F 0 "#PWR0132" H 2550 900 50  0001 C CNN
F 1 "+5V" H 2565 1223 50  0000 C CNN
F 2 "" H 2550 1050 50  0001 C CNN
F 3 "" H 2550 1050 50  0001 C CNN
	1    2550 1050
	1    0    0    -1  
$EndComp
Wire Notes Line
	550  5200 550  550 
Wire Notes Line
	550  550  4350 550 
Wire Notes Line
	4350 550  4350 5200
Wire Notes Line
	4350 5200 550  5200
$Comp
L power:+3.3V #PWR0133
U 1 1 5FF124E8
P 2750 1050
F 0 "#PWR0133" H 2750 900 50  0001 C CNN
F 1 "+3.3V" H 2765 1223 50  0000 C CNN
F 2 "" H 2750 1050 50  0001 C CNN
F 3 "" H 2750 1050 50  0001 C CNN
	1    2750 1050
	1    0    0    -1  
$EndComp
Text Notes 600  750  0    118  ~ 0
ESP32 wireless module
Text GLabel 1750 2150 0    50   Output ~ 0
LCD_LED
Wire Wire Line
	1750 2150 1950 2150
Text GLabel 1750 2900 0    50   BiDi ~ 0
ESP32_SDA
Text GLabel 1750 3000 0    50   Output ~ 0
ESP32_SCL
Wire Wire Line
	2750 1050 2750 1100
NoConn ~ 1950 4050
NoConn ~ 1950 4150
NoConn ~ 1950 4250
NoConn ~ 1950 4350
NoConn ~ 1950 4450
NoConn ~ 1950 4550
Text GLabel 3600 2400 2    50   Output ~ 0
LCD_~CE
Text GLabel 3600 2550 2    50   Output ~ 0
LCD_CLK
Text GLabel 3600 2750 2    50   Output ~ 0
LCD_DIN
Text GLabel 3600 2100 2    50   Output ~ 0
LCD_~RST
Text GLabel 3600 2250 2    50   Output ~ 0
LCD_DATA-~CMD
Text GLabel 1750 2250 0    50   Input ~ 0
ROT_SW
Text GLabel 3600 3050 2    50   Input ~ 0
ROT_A
Wire Wire Line
	3350 3050 3600 3050
Text GLabel 3600 3150 2    50   Input ~ 0
ROT_B
Wire Wire Line
	3600 3150 3350 3150
$Comp
L MCU_Module-extra:ESP32-DevKitC A1
U 1 1 5FF09372
P 2650 2950
F 0 "A1" H 3150 4200 50  0000 C CNN
F 1 "2x Conn_01x19_Female" H 3150 1200 50  0000 C CNN
F 2 "Module-extra:ESP32-DevKitC" H 2650 1050 50  0001 C CNN
F 3 "https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32_datasheet_en.pdf" H 2350 2850 50  0001 C CNN
F 4 "ESP32-DevKitC v4 Wroom 16MB" H 3300 1100 50  0000 C CNN "Description"
	1    2650 2950
	1    0    0    -1  
$EndComp
NoConn ~ 3350 2650
Wire Wire Line
	3450 2150 3450 2100
Wire Wire Line
	3450 2100 3600 2100
Wire Wire Line
	3350 2150 3450 2150
Wire Wire Line
	3350 2250 3600 2250
Wire Wire Line
	3350 2350 3450 2350
Wire Wire Line
	3450 2350 3450 2400
Wire Wire Line
	3450 2400 3600 2400
Wire Wire Line
	3350 2550 3600 2550
Wire Wire Line
	3350 2750 3600 2750
Wire Wire Line
	1750 2250 1950 2250
Wire Wire Line
	2650 4750 2650 4850
Wire Wire Line
	3350 3750 3550 3750
$Comp
L MCU_Microchip_SAMD-extra:SAMD21G_ARDUINO_M0 U1
U 1 1 60312A43
P 9250 3000
F 0 "U1" H 9250 4667 50  0000 C CNN
F 1 "ATSAMD21G18A-AU" H 9250 4576 50  0000 C CNN
F 2 "Package_QFP:TQFP-48_7x7mm_P0.5mm" H 9250 2800 50  0001 L CNN
F 3 "" H 9250 2800 50  0001 L CNN
	1    9250 3000
	1    0    0    -1  
$EndComp
Wire Notes Line
	4450 550  4450 5200
Wire Notes Line
	4450 5200 11150 5200
Wire Notes Line
	11150 550  4450 550 
Text Notes 4500 750  0    118  ~ 0
Heat controller
$Comp
L power:GND #PWR0152
U 1 1 6031BAF6
P 8150 4850
F 0 "#PWR0152" H 8150 4600 50  0001 C CNN
F 1 "GND" H 8155 4677 50  0000 C CNN
F 2 "" H 8150 4850 50  0001 C CNN
F 3 "" H 8150 4850 50  0001 C CNN
	1    8150 4850
	1    0    0    -1  
$EndComp
Text GLabel 10500 3300 2    50   Output ~ 0
HEAT_SCL
Wire Wire Line
	10500 3300 10250 3300
Text GLabel 10500 3200 2    50   BiDi ~ 0
HEAT_SDA
$Comp
L Device:R R?
U 1 1 60320633
P 1850 6350
AR Path="/604268EE/60320633" Ref="R?"  Part="1" 
AR Path="/60488463/60320633" Ref="R55"  Part="1" 
F 0 "R55" H 1780 6304 50  0000 R CNN
F 1 "4k7" H 1780 6395 50  0000 R CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 1780 6350 50  0001 C CNN
F 3 "~" H 1850 6350 50  0001 C CNN
	1    1850 6350
	-1   0    0    1   
$EndComp
$Comp
L Device:R R?
U 1 1 6032271B
P 2150 6350
AR Path="/604268EE/6032271B" Ref="R?"  Part="1" 
AR Path="/60488463/6032271B" Ref="R56"  Part="1" 
F 0 "R56" H 2080 6304 50  0000 R CNN
F 1 "4k7" H 2080 6395 50  0000 R CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 2080 6350 50  0001 C CNN
F 3 "~" H 2150 6350 50  0001 C CNN
	1    2150 6350
	-1   0    0    1   
$EndComp
$Comp
L power:+3.3V #PWR0157
U 1 1 6032601F
P 1850 5850
F 0 "#PWR0157" H 1850 5700 50  0001 C CNN
F 1 "+3.3V" H 1865 6023 50  0000 C CNN
F 2 "" H 1850 5850 50  0001 C CNN
F 3 "" H 1850 5850 50  0001 C CNN
	1    1850 5850
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR0158
U 1 1 60326C39
P 2150 5850
F 0 "#PWR0158" H 2150 5700 50  0001 C CNN
F 1 "+3.3V" H 2165 6023 50  0000 C CNN
F 2 "" H 2150 5850 50  0001 C CNN
F 3 "" H 2150 5850 50  0001 C CNN
	1    2150 5850
	1    0    0    -1  
$EndComp
Wire Wire Line
	1850 6200 1850 5850
Wire Wire Line
	2150 6200 2150 5850
$Comp
L Connector:USB_B_Micro J10
U 1 1 60328BFC
P 5200 3800
F 0 "J10" H 5350 4150 50  0000 C CNN
F 1 "USB_B_Micro" H 5450 3450 50  0000 C CNN
F 2 "Connector_USB-fixed:USB_Micro-B_Amphenol_10118194_Horizontal" H 5350 3750 50  0001 C CNN
F 3 "~" H 5350 3750 50  0001 C CNN
	1    5200 3800
	1    0    0    -1  
$EndComp
NoConn ~ 5500 3600
$Comp
L power:GND #PWR0149
U 1 1 6032BFDA
P 5200 4850
F 0 "#PWR0149" H 5200 4600 50  0001 C CNN
F 1 "GND" H 5205 4677 50  0000 C CNN
F 2 "" H 5200 4850 50  0001 C CNN
F 3 "" H 5200 4850 50  0001 C CNN
	1    5200 4850
	1    0    0    -1  
$EndComp
NoConn ~ 5500 4000
Wire Wire Line
	5500 3800 6050 3800
Wire Wire Line
	5500 3900 5650 3900
Wire Wire Line
	5650 3900 5650 3700
Wire Wire Line
	5650 3700 6500 3700
$Comp
L Diode:BAV99 D?
U 1 1 60335729
P 5850 4300
AR Path="/604268EE/60335729" Ref="D?"  Part="1" 
AR Path="/60488463/60335729" Ref="D3"  Part="1" 
F 0 "D3" V 5800 4150 50  0000 L CNN
F 1 "BAV99" V 5700 4000 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 5850 3800 50  0001 C CNN
F 3 "https://assets.nexperia.com/documents/data-sheet/BAV99_SER.pdf" H 5850 4300 50  0001 C CNN
	1    5850 4300
	0    -1   -1   0   
$EndComp
Wire Wire Line
	6050 4300 6050 3800
Connection ~ 6050 3800
Wire Wire Line
	6050 3800 8250 3800
Wire Wire Line
	6500 4300 6500 3700
Connection ~ 6500 3700
Wire Wire Line
	6500 3700 8250 3700
$Comp
L power:GND #PWR0150
U 1 1 603375D8
P 5850 4850
F 0 "#PWR0150" H 5850 4600 50  0001 C CNN
F 1 "GND" H 5855 4677 50  0000 C CNN
F 2 "" H 5850 4850 50  0001 C CNN
F 3 "" H 5850 4850 50  0001 C CNN
	1    5850 4850
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0151
U 1 1 6033779C
P 6300 4850
F 0 "#PWR0151" H 6300 4600 50  0001 C CNN
F 1 "GND" H 6305 4677 50  0000 C CNN
F 2 "" H 6300 4850 50  0001 C CNN
F 3 "" H 6300 4850 50  0001 C CNN
	1    6300 4850
	1    0    0    -1  
$EndComp
Wire Wire Line
	5850 4600 5850 4850
Wire Wire Line
	6300 4600 6300 4850
$Comp
L power:+3.3V #PWR0147
U 1 1 60339525
P 5850 3550
F 0 "#PWR0147" H 5850 3400 50  0001 C CNN
F 1 "+3.3V" H 5865 3723 50  0000 C CNN
F 2 "" H 5850 3550 50  0001 C CNN
F 3 "" H 5850 3550 50  0001 C CNN
	1    5850 3550
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR0148
U 1 1 6033A681
P 6300 3550
F 0 "#PWR0148" H 6300 3400 50  0001 C CNN
F 1 "+3.3V" H 6315 3723 50  0000 C CNN
F 2 "" H 6300 3550 50  0001 C CNN
F 3 "" H 6300 3550 50  0001 C CNN
	1    6300 3550
	1    0    0    -1  
$EndComp
Wire Wire Line
	6300 3550 6300 4000
Wire Wire Line
	5850 3550 5850 4000
$Comp
L Connector:Conn_ARM_JTAG_SWD_10 J11
U 1 1 6033CB6C
P 3100 6600
F 0 "J11" H 3450 7200 50  0000 R CNN
F 1 "Conn_ARM_JTAG_SWD_10" H 4100 6000 50  0000 R CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_2x05_P2.54mm_Vertical" H 3100 6600 50  0001 C CNN
F 3 "http://infocenter.arm.com/help/topic/com.arm.doc.ddi0314h/DDI0314H_coresight_components_trm.pdf" V 2750 5350 50  0001 C CNN
	1    3100 6600
	1    0    0    -1  
$EndComp
Wire Notes Line
	2500 7700 2500 5300
Text Notes 2550 5500 0    118  ~ 0
Heat controller SWD
$Comp
L power:GND #PWR0162
U 1 1 603425AD
P 3000 7400
F 0 "#PWR0162" H 3000 7150 50  0001 C CNN
F 1 "GND" H 3005 7227 50  0000 C CNN
F 2 "" H 3000 7400 50  0001 C CNN
F 3 "" H 3000 7400 50  0001 C CNN
	1    3000 7400
	1    0    0    -1  
$EndComp
Wire Wire Line
	3000 7400 3000 7300
Wire Wire Line
	3100 7200 3100 7300
Wire Wire Line
	3100 7300 3000 7300
Connection ~ 3000 7300
Wire Wire Line
	3000 7300 3000 7200
$Comp
L power:+3.3V #PWR0154
U 1 1 60344D3D
P 3100 5850
F 0 "#PWR0154" H 3100 5700 50  0001 C CNN
F 1 "+3.3V" H 3115 6023 50  0000 C CNN
F 2 "" H 3100 5850 50  0001 C CNN
F 3 "" H 3100 5850 50  0001 C CNN
	1    3100 5850
	1    0    0    -1  
$EndComp
Wire Wire Line
	3100 5850 3100 6000
Text GLabel 7850 1600 0    50   Input ~ 0
HEAT_~RESET
Text GLabel 3800 6300 2    50   Input ~ 0
HEAT_~RESET
Wire Wire Line
	3600 6300 3800 6300
NoConn ~ 3600 6800
NoConn ~ 3600 6700
Wire Wire Line
	3600 6500 3800 6500
Wire Wire Line
	3600 6600 3800 6600
Text GLabel 3800 6500 2    50   Output ~ 0
SWCLK
Text GLabel 7850 4000 0    50   Input ~ 0
SWCLK
Text GLabel 7850 4100 0    50   BiDi ~ 0
SWDIO
Text GLabel 3800 6600 2    50   BiDi ~ 0
SWDIO
$Comp
L Device:C C?
U 1 1 6035A716
P 10400 6000
AR Path="/6029EF83/6035A716" Ref="C?"  Part="1" 
AR Path="/60488463/6035A716" Ref="C38"  Part="1" 
F 0 "C38" H 10515 6046 50  0000 L CNN
F 1 "100nF" H 10515 5955 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder" H 10438 5850 50  0001 C CNN
F 3 "~" H 10400 6000 50  0001 C CNN
	1    10400 6000
	1    0    0    -1  
$EndComp
$Comp
L Device:R R?
U 1 1 6035BB84
P 9950 5800
AR Path="/604268EE/6035BB84" Ref="R?"  Part="1" 
AR Path="/60488463/6035BB84" Ref="R20"  Part="1" 
F 0 "R20" V 10157 5800 50  0000 C CNN
F 1 "470R" V 10066 5800 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 9880 5800 50  0001 C CNN
F 3 "~" H 9950 5800 50  0001 C CNN
	1    9950 5800
	0    -1   -1   0   
$EndComp
Wire Notes Line
	8850 5300 8850 6450
Wire Notes Line
	8850 6450 11150 6450
Wire Notes Line
	11150 6450 11150 5300
Wire Notes Line
	11150 5300 8850 5300
Text Notes 8900 5500 0    118  ~ 0
Heat controller reset
Wire Wire Line
	10100 5800 10400 5800
Wire Wire Line
	10400 5800 10400 5850
$Comp
L power:GND #PWR0160
U 1 1 603662F8
P 10400 6200
F 0 "#PWR0160" H 10400 5950 50  0001 C CNN
F 1 "GND" H 10405 6027 50  0000 C CNN
F 2 "" H 10400 6200 50  0001 C CNN
F 3 "" H 10400 6200 50  0001 C CNN
	1    10400 6200
	1    0    0    -1  
$EndComp
Wire Wire Line
	10400 6200 10400 6150
Text GLabel 10600 5800 2    50   Output ~ 0
HEAT_~RESET
Wire Wire Line
	10400 5800 10600 5800
Connection ~ 10400 5800
$Comp
L Switch:SW_Push SW2
U 1 1 6036AE7F
P 9450 5800
F 0 "SW2" H 9450 5707 50  0000 C CNN
F 1 "SW_Push" H 9450 5616 50  0000 C CNN
F 2 "Button_Switch_SMD:SW_SPST_CK_RS282G05A3" H 9450 6000 50  0001 C CNN
F 3 "~" H 9450 6000 50  0001 C CNN
	1    9450 5800
	1    0    0    -1  
$EndComp
Wire Wire Line
	9650 5800 9800 5800
$Comp
L power:GND #PWR0159
U 1 1 6036CC85
P 9000 6200
F 0 "#PWR0159" H 9000 5950 50  0001 C CNN
F 1 "GND" H 9005 6027 50  0000 C CNN
F 2 "" H 9000 6200 50  0001 C CNN
F 3 "" H 9000 6200 50  0001 C CNN
	1    9000 6200
	1    0    0    -1  
$EndComp
Wire Wire Line
	9250 5800 9000 5800
Wire Wire Line
	9000 5800 9000 6200
$Comp
L Device:Crystal_GND3 Y1
U 1 1 6036F13A
P 7050 2650
F 0 "Y1" H 7050 2918 50  0000 C CNN
F 1 "32768Hz" H 7050 2827 50  0000 C CNN
F 2 "Crystal:Crystal_C26-LF_D2.1mm_L6.5mm_Horizontal_1EP_style1" H 7050 2650 50  0001 C CNN
F 3 "~" H 7050 2650 50  0001 C CNN
	1    7050 2650
	1    0    0    -1  
$EndComp
$Comp
L Device:C C?
U 1 1 6037167D
P 7300 2900
AR Path="/6029EF83/6037167D" Ref="C?"  Part="1" 
AR Path="/60488463/6037167D" Ref="C2"  Part="1" 
F 0 "C2" H 7415 2946 50  0000 L CNN
F 1 "6.8pF" H 7415 2855 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder" H 7338 2750 50  0001 C CNN
F 3 "~" H 7300 2900 50  0001 C CNN
	1    7300 2900
	1    0    0    -1  
$EndComp
$Comp
L Device:C C?
U 1 1 60371DFA
P 6800 2900
AR Path="/6029EF83/60371DFA" Ref="C?"  Part="1" 
AR Path="/60488463/60371DFA" Ref="C1"  Part="1" 
F 0 "C1" H 6686 2946 50  0000 R CNN
F 1 "6.8pF" H 6686 2855 50  0000 R CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder" H 6838 2750 50  0001 C CNN
F 3 "~" H 6800 2900 50  0001 C CNN
	1    6800 2900
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0146
U 1 1 60372032
P 7050 3100
F 0 "#PWR0146" H 7050 2850 50  0001 C CNN
F 1 "GND" H 7055 2927 50  0000 C CNN
F 2 "" H 7050 3100 50  0001 C CNN
F 3 "" H 7050 3100 50  0001 C CNN
	1    7050 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	6900 2650 6800 2650
Wire Wire Line
	6800 2650 6800 2750
Wire Wire Line
	7200 2650 7300 2650
Wire Wire Line
	7300 2650 7300 2750
Wire Wire Line
	7300 3050 7050 3050
Wire Wire Line
	7050 3050 7050 3100
Wire Wire Line
	6800 3050 7050 3050
Connection ~ 7050 3050
$Comp
L Device:C C?
U 1 1 6038E2CD
P 5950 2750
AR Path="/6029EF83/6038E2CD" Ref="C?"  Part="1" 
AR Path="/60488463/6038E2CD" Ref="C41"  Part="1" 
F 0 "C41" H 5950 2950 50  0000 L CNN
F 1 "1uF" H 5950 2850 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder" H 5988 2600 50  0001 C CNN
F 3 "~" H 5950 2750 50  0001 C CNN
	1    5950 2750
	1    0    0    -1  
$EndComp
$Comp
L Device:C C?
U 1 1 6038EAD0
P 5700 2750
AR Path="/6029EF83/6038EAD0" Ref="C?"  Part="1" 
AR Path="/60488463/6038EAD0" Ref="C40"  Part="1" 
F 0 "C40" H 5700 2950 50  0000 L CNN
F 1 "1uF" H 5700 2850 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder" H 5738 2600 50  0001 C CNN
F 3 "~" H 5700 2750 50  0001 C CNN
	1    5700 2750
	1    0    0    -1  
$EndComp
$Comp
L Device:C C?
U 1 1 6038EFDD
P 5150 2750
AR Path="/6029EF83/6038EFDD" Ref="C?"  Part="1" 
AR Path="/60488463/6038EFDD" Ref="C35"  Part="1" 
F 0 "C35" H 5150 2950 50  0000 L CNN
F 1 "100nF" H 5150 2850 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder" H 5188 2600 50  0001 C CNN
F 3 "~" H 5150 2750 50  0001 C CNN
	1    5150 2750
	1    0    0    -1  
$EndComp
$Comp
L Device:C C?
U 1 1 6039145A
P 5450 2750
AR Path="/6029EF83/6039145A" Ref="C?"  Part="1" 
AR Path="/60488463/6039145A" Ref="C36"  Part="1" 
F 0 "C36" H 5450 2950 50  0000 L CNN
F 1 "100nF" H 5450 2850 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder" H 5488 2600 50  0001 C CNN
F 3 "~" H 5450 2750 50  0001 C CNN
	1    5450 2750
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0140
U 1 1 603935AD
P 5150 3100
F 0 "#PWR0140" H 5150 2850 50  0001 C CNN
F 1 "GND" H 5155 2927 50  0000 C CNN
F 2 "" H 5150 3100 50  0001 C CNN
F 3 "" H 5150 3100 50  0001 C CNN
	1    5150 3100
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0141
U 1 1 60393767
P 5450 3100
F 0 "#PWR0141" H 5450 2850 50  0001 C CNN
F 1 "GND" H 5455 2927 50  0000 C CNN
F 2 "" H 5450 3100 50  0001 C CNN
F 3 "" H 5450 3100 50  0001 C CNN
	1    5450 3100
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0142
U 1 1 6039395D
P 5700 3100
F 0 "#PWR0142" H 5700 2850 50  0001 C CNN
F 1 "GND" H 5705 2927 50  0000 C CNN
F 2 "" H 5700 3100 50  0001 C CNN
F 3 "" H 5700 3100 50  0001 C CNN
	1    5700 3100
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0143
U 1 1 60393B56
P 5950 3100
F 0 "#PWR0143" H 5950 2850 50  0001 C CNN
F 1 "GND" H 5955 2927 50  0000 C CNN
F 2 "" H 5950 3100 50  0001 C CNN
F 3 "" H 5950 3100 50  0001 C CNN
	1    5950 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	5150 3100 5150 2900
Wire Wire Line
	5450 3100 5450 2900
Wire Wire Line
	5700 3100 5700 2900
Wire Wire Line
	5950 3100 5950 2900
Wire Wire Line
	5950 2000 5950 2600
Wire Wire Line
	8250 1900 8150 1900
Wire Wire Line
	5700 1900 5700 2600
Wire Wire Line
	5700 1900 5450 1900
Wire Wire Line
	5450 1900 5450 2600
Connection ~ 5700 1900
Wire Wire Line
	5150 1800 5150 2600
Wire Wire Line
	8250 2100 8150 2100
Wire Wire Line
	8150 2100 8150 1900
$Comp
L power:+3.3VA #PWR0134
U 1 1 603A87FF
P 5150 1300
F 0 "#PWR0134" H 5150 1150 50  0001 C CNN
F 1 "+3.3VA" H 5165 1473 50  0000 C CNN
F 2 "" H 5150 1300 50  0001 C CNN
F 3 "" H 5150 1300 50  0001 C CNN
	1    5150 1300
	1    0    0    -1  
$EndComp
Wire Wire Line
	5150 1800 5150 1600
Connection ~ 5150 1800
$Comp
L power:+3.3V #PWR0135
U 1 1 603AF8E8
P 5450 1300
F 0 "#PWR0135" H 5450 1150 50  0001 C CNN
F 1 "+3.3V" H 5465 1473 50  0000 C CNN
F 2 "" H 5450 1300 50  0001 C CNN
F 3 "" H 5450 1300 50  0001 C CNN
	1    5450 1300
	1    0    0    -1  
$EndComp
Wire Wire Line
	5450 1900 5450 1600
Connection ~ 5450 1900
$Comp
L Device:C C?
U 1 1 603C632B
P 4900 2750
AR Path="/6029EF83/603C632B" Ref="C?"  Part="1" 
AR Path="/60488463/603C632B" Ref="C61"  Part="1" 
F 0 "C61" H 4900 2950 50  0000 L CNN
F 1 "10uF" H 4900 2850 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder" H 4938 2600 50  0001 C CNN
F 3 "~" H 4900 2750 50  0001 C CNN
	1    4900 2750
	1    0    0    -1  
$EndComp
$Comp
L Device:C C?
U 1 1 603C673A
P 4650 2750
AR Path="/6029EF83/603C673A" Ref="C?"  Part="1" 
AR Path="/60488463/603C673A" Ref="C60"  Part="1" 
F 0 "C60" H 4650 2950 50  0000 L CNN
F 1 "10uF" H 4650 2850 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder" H 4688 2600 50  0001 C CNN
F 3 "~" H 4650 2750 50  0001 C CNN
	1    4650 2750
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0139
U 1 1 603C69DA
P 4900 3100
F 0 "#PWR0139" H 4900 2850 50  0001 C CNN
F 1 "GND" H 4905 2927 50  0000 C CNN
F 2 "" H 4900 3100 50  0001 C CNN
F 3 "" H 4900 3100 50  0001 C CNN
	1    4900 3100
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0138
U 1 1 603C6B59
P 4650 3100
F 0 "#PWR0138" H 4650 2850 50  0001 C CNN
F 1 "GND" H 4655 2927 50  0000 C CNN
F 2 "" H 4650 3100 50  0001 C CNN
F 3 "" H 4650 3100 50  0001 C CNN
	1    4650 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	4650 2900 4650 3100
Wire Wire Line
	4900 2900 4900 3100
Wire Wire Line
	4900 2600 4900 1800
Wire Wire Line
	4900 1800 5150 1800
Wire Wire Line
	4650 1900 4650 2600
Wire Wire Line
	4650 1900 5450 1900
$Comp
L Device:L_Small L2
U 1 1 603D6486
P 5300 1600
F 0 "L2" V 5485 1600 50  0000 C CNN
F 1 "10uH" V 5394 1600 50  0000 C CNN
F 2 "Inductor_SMD:L_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 5300 1600 50  0001 C CNN
F 3 "~" H 5300 1600 50  0001 C CNN
	1    5300 1600
	0    -1   -1   0   
$EndComp
Wire Wire Line
	5400 1600 5450 1600
Connection ~ 5450 1600
Wire Wire Line
	5450 1600 5450 1300
Wire Wire Line
	5200 1600 5150 1600
Connection ~ 5150 1600
Wire Wire Line
	5150 1600 5150 1300
$Comp
L power:PWR_FLAG #FLG0108
U 1 1 603DDD5B
P 4800 1300
F 0 "#FLG0108" H 4800 1375 50  0001 C CNN
F 1 "PWR_FLAG" H 4800 1473 50  0000 C CNN
F 2 "" H 4800 1300 50  0001 C CNN
F 3 "~" H 4800 1300 50  0001 C CNN
	1    4800 1300
	1    0    0    -1  
$EndComp
Wire Wire Line
	5150 1600 4800 1600
Wire Wire Line
	4800 1600 4800 1300
Text GLabel 10500 4000 2    50   Input ~ 0
IRON_CURRENT
Wire Wire Line
	10500 4000 10250 4000
Text GLabel 10500 3900 2    50   Input ~ 0
VIN
Wire Wire Line
	10500 3900 10250 3900
$Comp
L power:GND #PWR?
U 1 1 60485902
P 5150 7400
AR Path="/604A18A1/60485902" Ref="#PWR?"  Part="1" 
AR Path="/60488463/60485902" Ref="#PWR0161"  Part="1" 
F 0 "#PWR0161" H 5150 7150 50  0001 C CNN
F 1 "GND" H 5155 7227 50  0000 C CNN
F 2 "" H 5150 7400 50  0001 C CNN
F 3 "" H 5150 7400 50  0001 C CNN
	1    5150 7400
	1    0    0    -1  
$EndComp
$Comp
L Device:R R?
U 1 1 60485908
P 5150 6500
AR Path="/604A18A1/60485908" Ref="R?"  Part="1" 
AR Path="/60488463/60485908" Ref="R57"  Part="1" 
F 0 "R57" H 5080 6454 50  0000 R CNN
F 1 "4k7" H 5080 6545 50  0000 R CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 5080 6500 50  0001 C CNN
F 3 "~" H 5150 6500 50  0001 C CNN
	1    5150 6500
	-1   0    0    1   
$EndComp
$Comp
L Device:LED D?
U 1 1 6048590E
P 5150 6950
AR Path="/604A18A1/6048590E" Ref="D?"  Part="1" 
AR Path="/60488463/6048590E" Ref="D10"  Part="1" 
F 0 "D10" V 5189 6832 50  0000 R CNN
F 1 "Amber" V 5098 6832 50  0000 R CNN
F 2 "LED_SMD:LED_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 5150 6950 50  0001 C CNN
F 3 "~" H 5150 6950 50  0001 C CNN
	1    5150 6950
	0    -1   -1   0   
$EndComp
Wire Notes Line
	5600 5300 5600 7700
Wire Notes Line
	4550 7700 4550 5300
Text GLabel 8150 2800 0    50   Output ~ 0
DEBUG_LED
Wire Wire Line
	8250 2800 8150 2800
Text GLabel 5150 6150 1    50   Input ~ 0
DEBUG_LED
Text Notes 4600 5500 0    118  ~ 0
Debug LED
Text GLabel 10500 2900 2    50   Output ~ 0
HEAT_PWM
Wire Wire Line
	10500 2900 10250 2900
Text GLabel 10500 3700 2    50   Input ~ 0
AIN_COMP
Text GLabel 10500 3600 2    50   Input ~ 0
TEMP
Wire Wire Line
	10250 3700 10400 3700
Wire Wire Line
	10250 3800 10400 3800
Wire Wire Line
	10400 3800 10400 3700
Connection ~ 10400 3700
Wire Wire Line
	10400 3700 10500 3700
Wire Wire Line
	5150 6150 5150 6350
Wire Wire Line
	5150 6650 5150 6800
Wire Wire Line
	5150 7100 5150 7400
Text GLabel 1750 2550 0    50   Output ~ 0
I2S_BCK
Text GLabel 1750 2650 0    50   Output ~ 0
I2S_WS
Wire Wire Line
	1750 2550 1950 2550
Wire Wire Line
	1950 2650 1750 2650
Text GLabel 3600 3350 2    50   Output ~ 0
I2S_DATA
Wire Wire Line
	3350 3350 3600 3350
Text GLabel 10500 3500 2    50   Input ~ 0
NTC
Text GLabel 10500 2100 2    50   Output ~ 0
HEAT_INT
Wire Wire Line
	10250 2100 10500 2100
Text GLabel 10500 2200 2    50   Output ~ 0
I2S_GAIN0
Text GLabel 10500 2300 2    50   Output ~ 0
I2S_GAIN1
Text GLabel 10500 2400 2    50   Output ~ 0
I2S_SD
Wire Wire Line
	10500 2200 10250 2200
Wire Wire Line
	10250 2300 10500 2300
Wire Wire Line
	10500 2400 10250 2400
Text Label 7800 3700 0    50   ~ 0
USB-
Text Label 7800 3800 0    50   ~ 0
USB+
Text Label 7750 1900 0    50   ~ 0
VDDIO
Text Label 7750 2000 0    50   ~ 0
VDDCORE
$Comp
L Device:CP C?
U 1 1 608DD07A
P 2950 1300
AR Path="/604268EE/608DD07A" Ref="C?"  Part="1" 
AR Path="/60488463/608DD07A" Ref="C73"  Part="1" 
F 0 "C73" H 3068 1346 50  0000 L CNN
F 1 "100uF/35V" H 3068 1255 50  0000 L CNN
F 2 "Capacitor_SMD:CP_Elec_6.3x7.7" H 2988 1150 50  0001 C CNN
F 3 "~" H 2950 1300 50  0001 C CNN
	1    2950 1300
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0137
U 1 1 608EAD5C
P 2950 1500
F 0 "#PWR0137" H 2950 1250 50  0001 C CNN
F 1 "GND" H 2955 1327 50  0000 C CNN
F 2 "" H 2950 1500 50  0001 C CNN
F 3 "" H 2950 1500 50  0001 C CNN
	1    2950 1500
	1    0    0    -1  
$EndComp
Wire Wire Line
	2950 1150 2950 1100
Wire Wire Line
	2950 1100 2750 1100
Connection ~ 2750 1100
Wire Wire Line
	2750 1100 2750 1650
Wire Wire Line
	2950 1500 2950 1450
NoConn ~ 7050 2850
NoConn ~ 3350 3950
NoConn ~ 3350 3850
Wire Notes Line
	2400 5300 2400 7700
Wire Notes Line
	550  7700 550  5300
Text GLabel 1100 7150 0    50   BiDi ~ 0
ESP32_SCL
Wire Wire Line
	1750 3000 1950 3000
Wire Wire Line
	1100 7150 1850 7150
Wire Wire Line
	1850 6500 1850 7150
Text GLabel 1100 7250 0    50   BiDi ~ 0
ESP32_SDA
Wire Wire Line
	1750 2900 1950 2900
Wire Wire Line
	1100 7250 2150 7250
Wire Wire Line
	2150 6500 2150 7250
NoConn ~ 3350 3650
Text GLabel 3550 3750 2    50   Input ~ 0
HEAT_INT
$Comp
L Diode:BAV99 D?
U 1 1 60334D21
P 6300 4300
AR Path="/604268EE/60334D21" Ref="D?"  Part="1" 
AR Path="/60488463/60334D21" Ref="D4"  Part="1" 
F 0 "D4" V 6250 4100 50  0000 L CNN
F 1 "BAV99" V 6150 4000 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 6300 3800 50  0001 C CNN
F 3 "https://assets.nexperia.com/documents/data-sheet/BAV99_SER.pdf" H 6300 4300 50  0001 C CNN
	1    6300 4300
	0    -1   -1   0   
$EndComp
Text GLabel 10500 1900 2    50   BiDi ~ 0
ESP32_SDA
Wire Wire Line
	10500 2000 10250 2000
$Comp
L Device:R R?
U 1 1 60317913
P 1250 6350
AR Path="/604268EE/60317913" Ref="R?"  Part="1" 
AR Path="/60488463/60317913" Ref="R53"  Part="1" 
F 0 "R53" H 1180 6304 50  0000 R CNN
F 1 "4k7" H 1180 6395 50  0000 R CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 1180 6350 50  0001 C CNN
F 3 "~" H 1250 6350 50  0001 C CNN
	1    1250 6350
	-1   0    0    1   
$EndComp
$Comp
L Device:R R?
U 1 1 60317E31
P 1550 6350
AR Path="/604268EE/60317E31" Ref="R?"  Part="1" 
AR Path="/60488463/60317E31" Ref="R54"  Part="1" 
F 0 "R54" H 1480 6304 50  0000 R CNN
F 1 "4k7" H 1480 6395 50  0000 R CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 1480 6350 50  0001 C CNN
F 3 "~" H 1550 6350 50  0001 C CNN
	1    1550 6350
	-1   0    0    1   
$EndComp
$Comp
L power:+3.3V #PWR0155
U 1 1 60317E3B
P 1250 5850
F 0 "#PWR0155" H 1250 5700 50  0001 C CNN
F 1 "+3.3V" H 1265 6023 50  0000 C CNN
F 2 "" H 1250 5850 50  0001 C CNN
F 3 "" H 1250 5850 50  0001 C CNN
	1    1250 5850
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR0156
U 1 1 60317E45
P 1550 5850
F 0 "#PWR0156" H 1550 5700 50  0001 C CNN
F 1 "+3.3V" H 1565 6023 50  0000 C CNN
F 2 "" H 1550 5850 50  0001 C CNN
F 3 "" H 1550 5850 50  0001 C CNN
	1    1550 5850
	1    0    0    -1  
$EndComp
Wire Wire Line
	1250 6200 1250 5850
Wire Wire Line
	1550 6200 1550 5850
Text GLabel 1100 7050 0    50   Output ~ 0
HEAT_SCL
Text GLabel 1100 6950 0    50   BiDi ~ 0
HEAT_SDA
Wire Wire Line
	1100 6950 1250 6950
Wire Wire Line
	1250 6950 1250 6500
Wire Wire Line
	1100 7050 1550 7050
Wire Wire Line
	1550 7050 1550 6500
Text Notes 600  5500 0    118  ~ 0
The pull is strong
Text GLabel 10500 2000 2    50   Input ~ 0
ESP32_SCL
Wire Notes Line
	11150 5200 11150 550 
Wire Wire Line
	10500 3200 10250 3200
Wire Wire Line
	7850 1600 8250 1600
Wire Wire Line
	7850 4000 8250 4000
Wire Wire Line
	7850 4100 8250 4100
Wire Wire Line
	5150 1800 8250 1800
Wire Wire Line
	10500 3600 10250 3600
Wire Wire Line
	10500 3500 10250 3500
Wire Wire Line
	10500 1900 10250 1900
NoConn ~ 8250 3600
NoConn ~ 8250 3400
NoConn ~ 8250 3300
NoConn ~ 8250 3200
$Comp
L Device:C C?
U 1 1 6047C861
P 6450 2750
AR Path="/6029EF83/6047C861" Ref="C?"  Part="1" 
AR Path="/60488463/6047C861" Ref="C37"  Part="1" 
F 0 "C37" H 6450 2950 50  0000 L CNN
F 1 "100nF" H 6450 2850 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder" H 6488 2600 50  0001 C CNN
F 3 "~" H 6450 2750 50  0001 C CNN
	1    6450 2750
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0145
U 1 1 6047CC9F
P 6450 3100
F 0 "#PWR0145" H 6450 2850 50  0001 C CNN
F 1 "GND" H 6455 2927 50  0000 C CNN
F 2 "" H 6450 3100 50  0001 C CNN
F 3 "" H 6450 3100 50  0001 C CNN
	1    6450 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	6450 3100 6450 2900
$Comp
L Device:C C?
U 1 1 6048B387
P 6200 2750
AR Path="/6029EF83/6048B387" Ref="C?"  Part="1" 
AR Path="/60488463/6048B387" Ref="C50"  Part="1" 
F 0 "C50" H 6200 2950 50  0000 L CNN
F 1 "10uF" H 6200 2850 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder" H 6238 2600 50  0001 C CNN
F 3 "~" H 6200 2750 50  0001 C CNN
	1    6200 2750
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0144
U 1 1 6048B90A
P 6200 3100
F 0 "#PWR0144" H 6200 2850 50  0001 C CNN
F 1 "GND" H 6205 2927 50  0000 C CNN
F 2 "" H 6200 3100 50  0001 C CNN
F 3 "" H 6200 3100 50  0001 C CNN
	1    6200 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	6200 3100 6200 2900
Wire Wire Line
	5700 1900 8150 1900
Connection ~ 8150 1900
Wire Wire Line
	5950 2000 8250 2000
Wire Wire Line
	8250 2200 6450 2200
Wire Wire Line
	6450 2200 6450 2600
Wire Wire Line
	6200 2200 6450 2200
Wire Wire Line
	6200 2200 6200 2600
Connection ~ 6450 2200
Wire Wire Line
	8250 2400 6800 2400
Wire Wire Line
	6800 2400 6800 2650
Connection ~ 6800 2650
Wire Wire Line
	8250 2500 7300 2500
Wire Wire Line
	7300 2500 7300 2650
Connection ~ 7300 2650
Text Label 7750 2200 0    50   ~ 0
AREF
Text Label 7750 2400 0    50   ~ 0
XIN32
Text Label 7750 2500 0    50   ~ 0
XOUT32
Wire Wire Line
	8250 4400 8150 4400
NoConn ~ 8250 3000
NoConn ~ 8250 2900
NoConn ~ 10250 4200
NoConn ~ 10250 4300
NoConn ~ 10250 4400
Wire Wire Line
	8150 4850 8150 4400
Text GLabel 10500 2600 2    50   Input ~ 0
ADC_RDY
Wire Wire Line
	10250 2600 10500 2600
Wire Wire Line
	8250 4300 8150 4300
Wire Wire Line
	8150 4300 8150 4400
Connection ~ 8150 4400
$Comp
L Memory_EEPROM:24LC32 U6
U 1 1 6035843C
P 6200 6550
F 0 "U6" H 6450 6800 50  0000 C CNN
F 1 "AT24C32" H 6400 6300 50  0000 C CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 6200 6550 50  0001 C CNN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/21072G.pdf" H 6200 6550 50  0001 C CNN
	1    6200 6550
	1    0    0    -1  
$EndComp
Wire Notes Line
	8750 6450 6950 6450
Wire Notes Line
	6950 6450 6950 5300
Wire Notes Line
	6950 5300 8750 5300
Wire Notes Line
	8750 5300 8750 6450
$Comp
L power:+3.3V #PWR01
U 1 1 60361699
P 6200 5850
F 0 "#PWR01" H 6200 5700 50  0001 C CNN
F 1 "+3.3V" H 6215 6023 50  0000 C CNN
F 2 "" H 6200 5850 50  0001 C CNN
F 3 "" H 6200 5850 50  0001 C CNN
	1    6200 5850
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR02
U 1 1 60361EAB
P 6200 7400
F 0 "#PWR02" H 6200 7150 50  0001 C CNN
F 1 "GND" H 6205 7227 50  0000 C CNN
F 2 "" H 6200 7400 50  0001 C CNN
F 3 "" H 6200 7400 50  0001 C CNN
	1    6200 7400
	1    0    0    -1  
$EndComp
Wire Notes Line
	5700 7700 5700 5300
Wire Notes Line
	5700 5300 6850 5300
Wire Notes Line
	6850 5300 6850 7700
Wire Notes Line
	6850 7700 5700 7700
Wire Wire Line
	6200 6850 6200 7400
Wire Wire Line
	6200 6250 6200 5850
NoConn ~ 6600 6650
NoConn ~ 5800 6650
NoConn ~ 5800 6550
NoConn ~ 5800 6450
Wire Notes Line
	4550 7700 5600 7700
Wire Notes Line
	5600 5300 4550 5300
Wire Notes Line
	2500 5300 4450 5300
Wire Notes Line
	2400 5300 550  5300
Wire Notes Line
	550  7700 2400 7700
Wire Notes Line
	2500 7700 4450 7700
Wire Notes Line
	4450 5300 4450 7700
Text Notes 5750 5500 0    118  ~ 0
EEPROM
Text GLabel 6750 6200 1    50   Input ~ 0
HEAT_SCL
Text GLabel 6650 6200 1    50   BiDi ~ 0
HEAT_SDA
Wire Wire Line
	6600 6450 6650 6450
Wire Wire Line
	6650 6450 6650 6200
Wire Wire Line
	6600 6550 6750 6550
Wire Wire Line
	6750 6550 6750 6200
NoConn ~ 1950 3300
NoConn ~ 1950 3200
NoConn ~ 10250 3000
NoConn ~ 3350 2050
$Comp
L Connector:TestPoint TP1
U 1 1 603DC263
P 10500 1600
F 0 "TP1" V 10500 1788 50  0000 L CNN
F 1 "TestPoint" V 10545 1788 50  0001 L CNN
F 2 "TestPoint-extra:TestPoint_Pad_D0.75mm" H 10700 1600 50  0001 C CNN
F 3 "~" H 10700 1600 50  0001 C CNN
	1    10500 1600
	0    1    1    0   
$EndComp
Wire Wire Line
	10250 1600 10500 1600
$Comp
L Connector:TestPoint TP2
U 1 1 603E3A1C
P 10500 1700
F 0 "TP2" V 10500 1888 50  0000 L CNN
F 1 "TestPoint" V 10545 1888 50  0001 L CNN
F 2 "TestPoint-extra:TestPoint_Pad_D0.75mm" H 10700 1700 50  0001 C CNN
F 3 "~" H 10700 1700 50  0001 C CNN
	1    10500 1700
	0    1    1    0   
$EndComp
Wire Wire Line
	10250 1700 10500 1700
$Comp
L Connector:TestPoint TP3
U 1 1 603F6C86
P 10500 2700
F 0 "TP3" V 10500 2888 50  0000 L CNN
F 1 "TestPoint" V 10545 2888 50  0001 L CNN
F 2 "TestPoint-extra:TestPoint_Pad_D0.75mm" H 10700 2700 50  0001 C CNN
F 3 "~" H 10700 2700 50  0001 C CNN
	1    10500 2700
	0    1    1    0   
$EndComp
Wire Wire Line
	10250 2700 10500 2700
$Comp
L Connector:TestPoint TP4
U 1 1 604116F7
P 10500 2800
F 0 "TP4" V 10500 2988 50  0000 L CNN
F 1 "TestPoint" V 10545 2988 50  0001 L CNN
F 2 "TestPoint-extra:TestPoint_Pad_D0.75mm" H 10700 2800 50  0001 C CNN
F 3 "~" H 10700 2800 50  0001 C CNN
	1    10500 2800
	0    1    1    0   
$EndComp
Wire Wire Line
	10250 2800 10500 2800
$Comp
L Connector:Conn_ARM_JTAG_SWD_10 J9
U 1 1 6045AF4E
P 1100 4150
F 0 "J9" H 1450 4750 50  0000 R CNN
F 1 "Conn_ARM_JTAG_SWD_10" H 2100 3550 50  0000 R CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_2x05_P2.54mm_Vertical" H 1100 4150 50  0001 C CNN
F 3 "http://infocenter.arm.com/help/topic/com.arm.doc.ddi0314h/DDI0314H_coresight_components_trm.pdf" V 750 2900 50  0001 C CNN
	1    1100 4150
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR04
U 1 1 6045D9E5
P 1000 4850
F 0 "#PWR04" H 1000 4600 50  0001 C CNN
F 1 "GND" H 1005 4677 50  0000 C CNN
F 2 "" H 1000 4850 50  0001 C CNN
F 3 "" H 1000 4850 50  0001 C CNN
	1    1000 4850
	1    0    0    -1  
$EndComp
Wire Wire Line
	1000 4850 1000 4800
Wire Wire Line
	1100 4750 1100 4800
Wire Wire Line
	1100 4800 1000 4800
Connection ~ 1000 4800
Wire Wire Line
	1000 4800 1000 4750
$Comp
L power:+3.3V #PWR03
U 1 1 60474EA3
P 1100 3400
F 0 "#PWR03" H 1100 3250 50  0001 C CNN
F 1 "+3.3V" H 1115 3573 50  0000 C CNN
F 2 "" H 1100 3400 50  0001 C CNN
F 3 "" H 1100 3400 50  0001 C CNN
	1    1100 3400
	1    0    0    -1  
$EndComp
Wire Wire Line
	1100 3400 1100 3550
Text GLabel 1200 1900 0    50   Input ~ 0
ESP32_~RESET
Wire Wire Line
	1200 1900 1400 1900
Wire Wire Line
	1950 3550 1700 3550
Wire Wire Line
	1700 3550 1700 4150
Wire Wire Line
	1700 4150 1600 4150
Wire Wire Line
	1950 3650 1650 3650
Wire Wire Line
	1650 3650 1650 4050
Wire Wire Line
	1650 4050 1600 4050
Wire Wire Line
	1950 3750 1750 3750
Wire Wire Line
	1750 3750 1750 4250
Wire Wire Line
	1750 4250 1600 4250
Wire Wire Line
	1600 4350 1800 4350
Wire Wire Line
	1800 4350 1800 3850
Wire Wire Line
	1800 3850 1950 3850
Wire Wire Line
	2550 1050 2550 1650
Text Label 10250 1600 0    50   ~ 0
D0
Text Label 10250 1700 0    50   ~ 0
D1
Text Label 10250 2700 0    50   ~ 0
D9
Text Label 10250 2800 0    50   ~ 0
D10
Text Notes 6250 6950 0    50   ~ 0
I2C: 0x50
Wire Wire Line
	1600 3150 1200 3150
Wire Wire Line
	1200 3150 1200 2000
Wire Wire Line
	1200 2000 1400 2000
Wire Wire Line
	1400 2000 1400 1900
Wire Wire Line
	1600 3150 1600 3850
Connection ~ 1400 1900
Wire Wire Line
	1400 1900 1950 1900
Wire Wire Line
	5200 4200 5200 4850
NoConn ~ 5100 4200
$EndSCHEMATC
