EESchema Schematic File Version 4
LIBS:laser_board-cache
EELAYER 26 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Laser Pulse Board"
Date "2018-12-18"
Rev "1"
Comp "Kansas State University Department of Physics"
Comment1 "Author: Jesse Laning"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Comparator:AD8561 U1
U 1 1 5C1CBA22
P 5700 3400
F 0 "U1" H 6041 3446 50  0000 L CNN
F 1 "AD8561" H 6041 3355 50  0000 L CNN
F 2 "Package_DIP:DIP-8_W7.62mm_Socket_LongPads" H 5700 3400 50  0001 C CNN
F 3 "https://www.analog.com/media/en/technical-documentation/data-sheets/ad8561.pdf" H 5700 3400 50  0001 C CNN
	1    5700 3400
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_Coaxial J2
U 1 1 5C193C4E
P 5100 3300
F 0 "J2" H 5000 3200 50  0000 L CNN
F 1 "Trigger In" H 5250 3300 50  0000 L CNN
F 2 "hapi:te_conn_bnc_1634503" H 5100 3300 50  0001 C CNN
F 3 " ~" H 5100 3300 50  0001 C CNN
	1    5100 3300
	-1   0    0    1   
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J1
U 1 1 5C193D52
P 5150 3850
F 0 "J1" H 5100 3950 50  0000 L CNN
F 1 "Power" H 4850 3650 50  0000 L CNN
F 2 "Connector_Molex:Molex_KK-254_AE-6410-02A_1x02_P2.54mm_Vertical" H 5150 3850 50  0001 C CNN
F 3 "~" H 5150 3850 50  0001 C CNN
	1    5150 3850
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0101
U 1 1 5C193E0F
P 5100 3000
F 0 "#PWR0101" H 5100 2750 50  0001 C CNN
F 1 "GND" V 5105 2872 50  0000 R CNN
F 2 "" H 5100 3000 50  0001 C CNN
F 3 "" H 5100 3000 50  0001 C CNN
	1    5100 3000
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR0102
U 1 1 5C193E35
P 4900 3950
F 0 "#PWR0102" H 4900 3700 50  0001 C CNN
F 1 "GND" V 4905 3822 50  0000 R CNN
F 2 "" H 4900 3950 50  0001 C CNN
F 3 "" H 4900 3950 50  0001 C CNN
	1    4900 3950
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR0103
U 1 1 5C193E5F
P 5600 3800
F 0 "#PWR0103" H 5600 3550 50  0001 C CNN
F 1 "GND" H 5605 3627 50  0000 C CNN
F 2 "" H 5600 3800 50  0001 C CNN
F 3 "" H 5600 3800 50  0001 C CNN
	1    5600 3800
	1    0    0    -1  
$EndComp
Wire Wire Line
	5700 3700 5700 3750
Wire Wire Line
	5700 3750 5600 3750
Wire Wire Line
	5600 3750 5600 3800
Wire Wire Line
	5600 3700 5600 3750
Connection ~ 5600 3750
$Comp
L Device:R_US R3
U 1 1 5C193FF1
P 5800 2900
F 0 "R3" H 5868 2946 50  0000 L CNN
F 1 "10k" H 5868 2855 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal" V 5840 2890 50  0001 C CNN
F 3 "~" H 5800 2900 50  0001 C CNN
	1    5800 2900
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0104
U 1 1 5C1941AB
P 5800 2700
F 0 "#PWR0104" H 5800 2450 50  0001 C CNN
F 1 "GND" H 5805 2527 50  0000 C CNN
F 2 "" H 5800 2700 50  0001 C CNN
F 3 "" H 5800 2700 50  0001 C CNN
	1    5800 2700
	-1   0    0    1   
$EndComp
Wire Wire Line
	5800 3100 5800 3050
Wire Wire Line
	5800 2750 5800 2700
Wire Wire Line
	4950 3950 4900 3950
$Comp
L power:VCC #PWR0105
U 1 1 5C1947B7
P 4900 3850
F 0 "#PWR0105" H 4900 3700 50  0001 C CNN
F 1 "VCC" V 4918 3977 50  0000 L CNN
F 2 "" H 4900 3850 50  0001 C CNN
F 3 "" H 4900 3850 50  0001 C CNN
	1    4900 3850
	0    -1   -1   0   
$EndComp
Wire Wire Line
	4950 3850 4900 3850
$Comp
L power:VCC #PWR0106
U 1 1 5C194849
P 5600 3000
F 0 "#PWR0106" H 5600 2850 50  0001 C CNN
F 1 "VCC" H 5617 3173 50  0000 C CNN
F 2 "" H 5600 3000 50  0001 C CNN
F 3 "" H 5600 3000 50  0001 C CNN
	1    5600 3000
	1    0    0    -1  
$EndComp
$Comp
L Device:R_US R2
U 1 1 5C1949F7
P 5350 3700
F 0 "R2" H 5418 3746 50  0000 L CNN
F 1 "10k" H 5418 3655 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal" V 5390 3690 50  0001 C CNN
F 3 "~" H 5350 3700 50  0001 C CNN
	1    5350 3700
	1    0    0    -1  
$EndComp
$Comp
L Device:R_US R1
U 1 1 5C194A5F
P 5150 3500
F 0 "R1" V 5050 3400 50  0000 C CNN
F 1 "10k" V 5250 3500 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal" V 5190 3490 50  0001 C CNN
F 3 "~" H 5150 3500 50  0001 C CNN
	1    5150 3500
	0    1    1    0   
$EndComp
$Comp
L power:VCC #PWR0107
U 1 1 5C194B06
P 4950 3500
F 0 "#PWR0107" H 4950 3350 50  0001 C CNN
F 1 "VCC" V 4968 3627 50  0000 L CNN
F 2 "" H 4950 3500 50  0001 C CNN
F 3 "" H 4950 3500 50  0001 C CNN
	1    4950 3500
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR0108
U 1 1 5C194D71
P 5350 3900
F 0 "#PWR0108" H 5350 3650 50  0001 C CNN
F 1 "GND" H 5355 3727 50  0000 C CNN
F 2 "" H 5350 3900 50  0001 C CNN
F 3 "" H 5350 3900 50  0001 C CNN
	1    5350 3900
	1    0    0    -1  
$EndComp
Wire Wire Line
	4950 3500 5000 3500
Wire Wire Line
	5300 3500 5350 3500
Wire Wire Line
	5350 3550 5350 3500
Connection ~ 5350 3500
Wire Wire Line
	5350 3500 5400 3500
Wire Wire Line
	5350 3900 5350 3850
$Comp
L hapi:12226-5150-00FR J5
U 1 1 5C194CEC
P 8400 2150
F 0 "J5" H 8450 2100 50  0000 L CNN
F 1 "12226-5150-00FR" H 7450 2200 50  0000 L CNN
F 2 "hapi:3mtm-shrunk-delta-ribbon-sdr-connectors-122-series-ts2124" H 7550 1900 50  0001 C CNN
F 3 "" H 7550 1900 50  0001 C CNN
	1    8400 2150
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0109
U 1 1 5C1953E9
P 7050 2200
F 0 "#PWR0109" H 7050 1950 50  0001 C CNN
F 1 "GND" V 7055 2072 50  0000 R CNN
F 2 "" H 7050 2200 50  0001 C CNN
F 3 "" H 7050 2200 50  0001 C CNN
	1    7050 2200
	0    1    1    0   
$EndComp
Wire Wire Line
	7050 2200 7050 2300
Wire Wire Line
	7050 2300 7150 2300
Wire Wire Line
	7050 2300 7050 2400
Wire Wire Line
	7050 2400 7150 2400
Connection ~ 7050 2300
Wire Wire Line
	7050 2400 7050 3000
Wire Wire Line
	7050 3600 7150 3600
Connection ~ 7050 2400
Wire Wire Line
	7050 3600 7050 3700
Wire Wire Line
	7050 3700 7150 3700
Connection ~ 7050 3600
Wire Wire Line
	7050 3700 7050 4300
Wire Wire Line
	7050 4700 7150 4700
Connection ~ 7050 3700
Wire Wire Line
	7050 4700 7050 4900
Wire Wire Line
	7050 4900 8300 4900
Wire Wire Line
	8300 4900 8300 4850
Connection ~ 7050 4700
$Comp
L Connector_Generic:Conn_01x02 J3
U 1 1 5C196E53
P 6750 3000
F 0 "J3" H 6670 2675 50  0000 C CNN
F 1 "Laser Ready" H 6670 2766 50  0000 C CNN
F 2 "Connector_Molex:Molex_KK-254_AE-6410-02A_1x02_P2.54mm_Vertical" H 6750 3000 50  0001 C CNN
F 3 "~" H 6750 3000 50  0001 C CNN
	1    6750 3000
	-1   0    0    1   
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J4
U 1 1 5C196F41
P 6750 4300
F 0 "J4" H 6670 3975 50  0000 C CNN
F 1 "Laser Fault" H 6670 4066 50  0000 C CNN
F 2 "Connector_Molex:Molex_KK-254_AE-6410-02A_1x02_P2.54mm_Vertical" H 6750 4300 50  0001 C CNN
F 3 "~" H 6750 4300 50  0001 C CNN
	1    6750 4300
	-1   0    0    1   
$EndComp
Wire Wire Line
	6950 4200 7150 4200
Wire Wire Line
	6950 4300 7050 4300
Connection ~ 7050 4300
Wire Wire Line
	7050 4300 7050 4700
Wire Wire Line
	6950 2900 7150 2900
Wire Wire Line
	6950 3000 7050 3000
Connection ~ 7050 3000
Wire Wire Line
	7050 3000 7050 3600
Wire Wire Line
	6000 3300 7150 3300
Wire Wire Line
	6000 3500 6050 3500
Wire Wire Line
	6050 3500 6050 4600
Wire Wire Line
	6050 4600 7150 4600
$Comp
L Device:CP1 C1
U 1 1 5C19BFED
P 5350 3050
F 0 "C1" V 5098 3050 50  0000 C CNN
F 1 "CP1" V 5189 3050 50  0000 C CNN
F 2 "Capacitor_THT:CP_Radial_D5.0mm_P2.50mm" H 5350 3050 50  0001 C CNN
F 3 "~" H 5350 3050 50  0001 C CNN
	1    5350 3050
	0    1    1    0   
$EndComp
Wire Wire Line
	5300 3300 5400 3300
Wire Wire Line
	5600 3000 5600 3050
Wire Wire Line
	5600 3050 5500 3050
Connection ~ 5600 3050
Wire Wire Line
	5600 3050 5600 3100
Wire Wire Line
	5100 3000 5100 3050
Wire Wire Line
	5100 3050 5200 3050
Connection ~ 5100 3050
Wire Wire Line
	5100 3050 5100 3100
Text GLabel 6050 3750 2    50   Input ~ 0
DP_P
Text GLabel 6400 3300 1    50   Input ~ 0
DP_N
$EndSCHEMATC
