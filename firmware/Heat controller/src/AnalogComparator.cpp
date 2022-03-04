/*

Copyright 2021-2022 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#include <Arduino.h>

#include "AnalogComparator.h"

void AC_Handler(void)
{
    if (AC->INTFLAG.reg & AC_INTFLAG_COMP0)
    {
        AC->INTFLAG.reg = AC_INTFLAG_COMP0;
        AnalogComparator.trigger();
    }
}

AnalogComparatorClass::AnalogComparatorClass()
{
}

void AnalogComparatorClass::trigger(void)
{
    this->triggered = true;
}

bool AnalogComparatorClass::hasTriggered(void)
{
    if (this->triggered == true)
    {
        this->triggered = false;

        return true;
    }

    return false;
}

void AnalogComparatorClass::init(void)
{
    //PA4 / A3 / AIN[0] / MUX_B

    PORT->Group[PORTA].PMUX[2].reg = PORT_PMUX_PMUXE_B;
    PORT->Group[PORTA].PINCFG[4].reg |= PORT_PINCFG_PMUXEN | PORT_PINCFG_INEN;

    PM->APBCMASK.reg |= PM_APBCMASK_AC;

    GCLK->CLKCTRL.reg =
        GCLK_CLKCTRL_ID(AC_GCLK_ID_ANA) |
        GCLK_CLKCTRL_CLKEN |
        GCLK_CLKCTRL_GEN(0);

    GCLK->CLKCTRL.reg =
        GCLK_CLKCTRL_ID(AC_GCLK_ID_DIG) |
        GCLK_CLKCTRL_CLKEN |
        GCLK_CLKCTRL_GEN(0);

    AC->CTRLA.reg = AC_CTRLA_SWRST;
    while (AC->CTRLA.bit.SWRST)
        ;

    AC->EVCTRL.reg = AC_EVCTRL_COMPEI0;
    while (AC->STATUSB.bit.SYNCBUSY)
        ;

    AC->COMPCTRL[0].reg =
        AC_COMPCTRL_ENABLE |
        AC_COMPCTRL_SPEED(AC_COMPCTRL_SPEED_HIGH_Val) |
        AC_COMPCTRL_INTSEL(AC_COMPCTRL_INTSEL_RISING_Val) |
        AC_COMPCTRL_HYST |
        AC_COMPCTRL_MUXNEG(AC_COMPCTRL_MUXNEG_VSCALE_Val) |
        AC_COMPCTRL_MUXPOS(AC_COMPCTRL_MUXPOS_PIN0_Val) |
        AC_COMPCTRL_OUT(AC_COMPCTRL_OUT_ASYNC_Val);
    while (AC->STATUSB.bit.SYNCBUSY)
        ;

    AC->SCALER[0].reg = AC_SCALER_VALUE(40);
    while (AC->STATUSB.bit.SYNCBUSY)
        ;

    AC->INTENSET.bit.COMP0 = 1;
    NVIC_EnableIRQ(AC_IRQn);

    AC->CTRLA.reg |= AC_CTRLA_ENABLE;
    while (AC->STATUSB.bit.SYNCBUSY)
        ;
}

void AnalogComparatorClass::loop(void)
{
    static uint16_t accounter = 0;
    if (hasTriggered())
    {
        accounter++;

        //SerialUSB.println("AC triggered " + String(accounter));
    }
}

AnalogComparatorClass AnalogComparator;