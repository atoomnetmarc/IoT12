/*

Copyright 2021 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#include <Arduino.h>
#include <Wire.h>

#include <NTC.h>
#include <Thermocouple.h>

#include "AdcExternal.h"
#include "AdcInternal.h"
#include "AnalogComparator.h"
#include "config.h"
#include "main.h"
#include "Sensor.h"
#include "Setting.h"
#include "WireMinion.h"

NTC ntc;
Thermocouple thermocouple;

volatile float tAmbient = 0;
volatile float tTip = 0;
volatile float vVINHeatOn = 0;
volatile float vVINHeatOff = 0;
volatile float iIronPowerOff = 0;
volatile float iIronPowerOn = 0;

void HardFault_Handler(void)
{
#if defined DEBUG
    __BKPT(3);
#endif
    NVIC_SystemReset();
    for (;;)
    {
    }
}

float readThermocoupleVoltage()
{
    float v = ADCInternal.readChannel(PIN_INTERNAL_ADC_TEMP);
    return v / Setting.gainHeaterTemperatureAmplifier;
}

float readVin()
{
    return ADCInternal.readChannel(PIN_INTERNAL_ADC_VIN) / Setting.gainVSupplyResistorDivider;
}

volatile bool eventHeatOff = false;
volatile bool eventHeatOn = false;

float frequency = 5;
#define DIV_TCC 1
uint32_t timeHeatingOffMeasureTempUs = 20000;
uint32_t timeHeatingCooldownUs = 1000;

uint32_t periodHeatAllowed;
uint32_t periodHeatDisabled;
volatile float heatfactor = 0;

void TCC0_Handler()
{
    static uint32_t periodHeatEnabled = 0;

    if (TCC0->INTFLAG.bit.CNT)
    {
        TCC0->INTFLAG.bit.CNT = 1;

        static uint8_t state = 0;

        if (state == 0)
        {
            //Current state is (possible) heating, next state is off for measure.
            if (periodHeatEnabled > 0)
                eventHeatOn = true;

            state = 1;
            digitalWrite(9, HIGH);
            TCC0->PERB.reg = periodHeatDisabled;
            while (TCC0->SYNCBUSY.bit.PERB)
                ;
            TCC0->CCB[2].reg = 0;
            while (TCC0->SYNCBUSY.bit.CCB2)
                ;
        }
        else
        {
            eventHeatOff = true;
            state = 0;
            digitalWrite(9, LOW);
            TCC0->PERB.reg = periodHeatAllowed;

            if (heatfactor > 1)
                heatfactor = 1;

            if (heatfactor < 0)
                heatfactor = 0;

            periodHeatEnabled = (periodHeatAllowed - (timeHeatingCooldownUs * (F_CPU / DIV_TCC / 1000000))) * heatfactor;
            while (TCC0->SYNCBUSY.bit.PERB)
                ;
            TCC0->CCB[2].reg = periodHeatEnabled;
            while (TCC0->SYNCBUSY.bit.CCB2)
                ;
        }
    }
}

void ConfigureTCC(void)
{
    //F: TCC0/WO[6], PA16, D11

    uint32_t totalPeriod = (F_CPU / DIV_TCC / frequency) - 1;

    //Max 24-bit period.
    if (totalPeriod > 1 << 24)
        totalPeriod = 1 << 24;

    periodHeatDisabled = timeHeatingOffMeasureTempUs * (F_CPU / DIV_TCC / 1000000);
    periodHeatAllowed = totalPeriod - periodHeatDisabled;

    // Enable and configure generic clock generator 4
    GCLK->GENCTRL.reg = GCLK_GENCTRL_IDC |         // Improve duty cycle
                        GCLK_GENCTRL_GENEN |       // Enable generic clock gen
                        GCLK_GENCTRL_SRC_DFLL48M | // Select 48MHz as source
                        GCLK_GENCTRL_ID(4);        // Select GCLK4
    while (GCLK->STATUS.bit.SYNCBUSY)
        ; // Wait for synchronization

    // Set clock divider of 1 to generic clock generator 4
    GCLK->GENDIV.reg = GCLK_GENDIV_DIV(DIV_TCC) | // Divide 48 MHz by 1
                       GCLK_GENDIV_ID(4);         // Apply to GCLK4 4
    while (GCLK->STATUS.bit.SYNCBUSY)
        ; // Wait for synchronization

    // Enable GCLK4 and connect it to TCC0 and TCC1
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN |       // Enable generic clock
                        GCLK_CLKCTRL_GEN_GCLK4 |   // Select GCLK4
                        GCLK_CLKCTRL_ID_TCC0_TCC1; // Feed GCLK4 to TCC0/1
    while (GCLK->STATUS.bit.SYNCBUSY)
        ; // Wait for synchronization

    // Divide counter by 1 giving 48 MHz (20.83 ns) on each TCC0 tick
    TCC0->CTRLA.reg |= TCC_CTRLA_PRESCALER(TCC_CTRLA_PRESCALER_DIV1_Val);

    // Use "Normal PWM" (single-slope PWM): count up to PER, match on CC[n]
    TCC0->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM; // Select NPWM as waveform
    while (TCC0->SYNCBUSY.bit.WAVE)
        ; // Wait for synchronization

    // Set the period (the number to count to (TOP) before resetting timer)
    TCC0->PER.reg = periodHeatAllowed;
    while (TCC0->SYNCBUSY.bit.PER)
        ;

    // Set PWM signal off
    // n for CC[n] is determined by n = x % 4 where x is from WO[x]
    TCC0->CC[2].reg = 0;
    while (TCC0->SYNCBUSY.bit.CC2)
        ;

    // Configure PA16 (D11 on Arduino M0) to be output
    PORT->Group[PORTA].DIRSET.reg = PORT_PA16; // Set pin as output
    PORT->Group[PORTA].OUTCLR.reg = PORT_PA16; // Set pin to low

    // Enable the port multiplexer for PA16
    PORT->Group[PORTA].PINCFG[16].reg |= PORT_PINCFG_PMUXEN;

    // Connect TCC0 timer to PA16. Function F is TCC0/WO[6] for PA16.
    // Odd pin num (2*n + 1): use PMUXO
    // Even pin num (2*n): use PMUXE
    PORT->Group[PORTA].PMUX[8].reg = PORT_PMUX_PMUXE_F;

    TCC0->INTENSET.reg =
        TCC_INTENSET_CNT;

    NVIC_EnableIRQ(TCC0_IRQn);

    // Enable output (start PWM)
    TCC0->CTRLA.reg |= (TCC_CTRLA_ENABLE);
    while (TCC0->SYNCBUSY.bit.ENABLE)
        ; // Wait for synchronization
}

void setup()
{
    pinMode(0, OUTPUT);  //TP1
    pinMode(1, OUTPUT);  //TP2
    pinMode(9, OUTPUT);  //TP3
    pinMode(10, OUTPUT); //TP4
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    SerialUSB.begin(115200);

    Wire.begin();
    Wire.setClock(100000);
    WireMinion.init();

    Sensor.init();
    Setting.init();
    I2SGPIO.init();

    ntc.SetParameters(3.3,
                      Setting.resistanceNTCPullUp,
                      10000,
                      25 + 273.15,
                      3950);

    ConfigureTCC();
    AnalogComparator.init();
}

void loop()
{
    Setting.loop();
    Sensor.loop();
    AnalogComparator.loop();
    WireMinion.loop();

    static uint16_t accounter = 0;
    if (AnalogComparator.hasTriggered())
    {
        accounter++;

        //SerialUSB.println("AC triggered " + String(accounter));
    }

    if (eventHeatOn)
    {
        delayMicroseconds(500);
        eventHeatOn = false;
        vVINHeatOn = readVin();
        iIronPowerOn = ADCInternal.readChannel(PIN_INTERNAL_ADC_IRON_CURRENT) / Setting.gainHeaterCurrentAmplifier / Setting.resistanceHeaterShunt;

        WireMinion.raiseInterrupt(INT_MEASUREMENTS_GROUP_B);
    }

    if (eventHeatOff)
    {
        eventHeatOff = false;

        unsigned long start = millis();
        float vNTC = ADCInternal.readChannel(PIN_INTERNAL_ADC_NTC);
        float vThermocouple = readThermocoupleVoltage();
        vVINHeatOff = readVin();
        iIronPowerOff = ADCInternal.readChannel(PIN_INTERNAL_ADC_IRON_CURRENT) / Setting.gainHeaterCurrentAmplifier / Setting.resistanceHeaterShunt;
        float vAIN = ADCInternal.readChannel(PIN_INTERNAL_ADC_AIN_COMP, 0);
        unsigned long stop = millis();

        //About 80% of the thermal energy arrives at the tip.
        float heatTransferCoreTipFactor = 0.82;

        float Tthermocouple = thermocouple.GetTemperature(vThermocouple);
        tAmbient = ntc.GetTemperature(vNTC);
        tTip = (Tthermocouple * heatTransferCoreTipFactor) + tAmbient;

        WireMinion.raiseInterrupt(INT_MEASUREMENTS_GROUP_A);

        SerialUSB.println(String("NTC: ") + String(vNTC, 3) + String("V"));
        SerialUSB.println(String("NTC: ") + String(tAmbient, 3) + String("K"));
        SerialUSB.println(String("NTC: ") + String(tAmbient - 273.15, 3) + String("°C"));
        SerialUSB.println(String("Thermocouple: ") + String(vThermocouple * 1000, 3) + String("mV"));
        SerialUSB.println(String("TemperatureTip: ") + String(tTip - 273.15) + String("°C"));
        SerialUSB.println(String("Target Temperature: ") + String(Setting.targetTemperatureHeater - 273.15) + String("°C"));
        SerialUSB.println(String("vVINHeatOff: ") + String(vVINHeatOff, 1) + String("V"));
        SerialUSB.println(String("vVINHeatOn: ") + String(vVINHeatOn, 1) + String("V"));
        SerialUSB.println(String("Iron current Off: ") + String(iIronPowerOff, 2) + String("A"));
        SerialUSB.println(String("Iron current On: ") + String(iIronPowerOn, 2) + String("A"));
        SerialUSB.println(String("VAIN: ") + String(vAIN, 3) + String("V"));

        SerialUSB.println(String("Rtip: ") + String(vVINHeatOn / iIronPowerOn, 2) + String("Ω"));
        SerialUSB.println(String("Rsuppy: ") + String((vVINHeatOff - vVINHeatOn) / iIronPowerOn, 2) + String("Ω"));

        SerialUSB.println(String("reading adc took ") + String(stop - start) + String("ms"));
        SerialUSB.println();

        ADCInternal.requestDump();
        ADCExternal.requestDump();

        if (Setting.targetTemperatureHeater > tTip && Setting.heaterState == HEATER_STATE::ON)
        {
            heatfactor = 0.2;
        }
        else
        {
            heatfactor = 0;
        }
    }
}
