/*

Copyright 2021 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#include <SPI.h>

#include <TaskManagerIO.h>
#include <U8g2lib.h>

#include <HeatController.h>

#include "config.h"
#include "display.h"
#include "main.h"

U8G2_PCD8544_84X48_F_4W_HW_SPI_CUSTOM u8g2(U8G2_R0, /* cs=*/PIN_LCD_CE, /* dc=*/PIN_LCD_DATACMD, /* reset=*/PIN_LCD_RST); // Nokia 5110 Display

volatile uint8_t requestDrawDisplay = 0;

void RequestDrawDisplay()
{
    requestDrawDisplay = 1;
}

void drawProgressBar(float progress, int pbX, int pbY, int pbW, int pbH)
{
    if (progress > 1)
        progress = 1;
    if (progress < 0)
        progress = 0;

    u8g2.drawFrame(pbX, pbY, pbW, pbH);
    u8g2.drawBox(pbX + 1, pbY + 1, (pbW - 2) * progress, pbH - 2);
}

void DrawDisplay()
{
    if (requestDrawDisplay == 0)
    {
        return;
    }
    requestDrawDisplay = 0;

    u8g2.clearBuffer(); // clear the internal memory

    u8g2.setFont(u8g2_font_7x13_tf); // choose a suitable font

    /*
    u8g2.drawUTF8(00, 11, (String(F("0: ")) + String(ADC.getVoltage(0)*1000, 5) + String("mV")).c_str());
    u8g2.drawUTF8(00, 23, (String(F("1: ")) + String(ADC.getVoltage(1), 6) + String("V")).c_str());
    u8g2.drawUTF8(00, 35, (String(F("2: ")) + String(ADC.getVoltage(2)*1000, 5) + String("mV")).c_str());
    */

    //u8g2.drawUTF8(00, 11, (String(y*1000, 5) + String("mV")).c_str());


    u8g2.drawUTF8(00, 11, (String(F("Ttip ")) + String(HeatController.getTemperatureTip() - 273.15, 1) + String("°C")).c_str());
    u8g2.drawUTF8(00, 23, (String(F("Tset ")) + String(HeatController.getTargetTemperature() - 273.15, 1) + String("°C")).c_str());
    u8g2.drawUTF8(00, 34, (String(F("Tamb ")) + String(HeatController.getTemperatureNTC() - 273.15, 2) + String(F("°C"))).c_str());
    if (HeatController.getHeaterState() == HEATER_STATE::ON)
        u8g2.drawUTF8(00, 45, (String(F("On"))).c_str());
    else
        u8g2.drawUTF8(00, 45, (String(F("Off"))).c_str());

    //u8g2.setFont(u8g2_font_6x10_tf);
    //u8g2.drawUTF8(00, 47, (String(Heater.voltagePowerSupplyUnloaded, 2) + String("V")).c_str());
    //u8g2.drawUTF8(83/2, 47, (String(Heater.voltagePowerSupplyLoaded, 2) + String("V")).c_str());
    //u8g2.drawUTF8(83/2, 47, (String(Heater.powerTipMax, 2) + String("W")).c_str());

    /*

    int f = (1.8 * encoderValue) + 32;
    sprintf(buffer, "%3d", f);
    u8g2.drawUTF8(00, 24, (String(buffer) + String(F("°F"))).c_str());

    int k = 273.15 + encoderValue;
    sprintf(buffer, "%3d", k);
    u8g2.drawUTF8(00, 36, (String(buffer) + String(F("K"))).c_str());

    //[°De] = (100 − [°C]) × ​3⁄2
    int d = (100 - encoderValue) * 1.5;
    sprintf(buffer, "%3d", d);
    u8g2.drawUTF8(0, 48, (String(buffer) + String(F("°D"))).c_str());

    //[°Rø] = [°C] × ​21⁄40 + 7.5
    int ro = encoderValue * 21 / 40 * 7.5;
    sprintf(buffer, "%3d", ro);
    u8g2.drawUTF8(42, 12, (String(buffer) + String(F("°Rø"))).c_str());

    //[°Ré] = [°C] × ​4⁄5
    int re = encoderValue * 4 / 5;
    sprintf(buffer, "%3d", re);
    u8g2.drawUTF8(42, 24, (String(buffer) + String(F("°Ré"))).c_str());

    //°R
    int ra = encoderValue * 1.8 + 491.67;
    sprintf(buffer, "%3d", ra);
    u8g2.drawUTF8(42, 36, (String(buffer) + String(F("°R"))).c_str());

    //°N
    int n = encoderValue * 33 / 100;
    sprintf(buffer, "%3d", n);
    u8g2.drawUTF8(42, 48, (String(buffer) + String(F("°N"))).c_str());
    */



    u8g2.sendBuffer();
}

void DisplaySetup()
{
    u8g2.setBusClock(8000000);

    u8g2.begin();
    u8g2.clearBuffer();
    u8g2.sendBuffer();

    const uint8_t fps = 25;
    taskManager.scheduleFixedRate(
        1000000 / fps, [] { DrawDisplay(); },
        TIME_MICROS);
}

void DisplayLoop()
{
}

//Custom U8G2 function to yield() to get less WiFi dropouts and keep up with user input.
extern "C" uint8_t u8x8_byte_arduino_hw_spi_custom(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    uint8_t *data;
    uint8_t internal_spi_mode;

    switch (msg)
    {
    case U8X8_MSG_BYTE_SEND:
        data = (uint8_t *)arg_ptr;
        while (arg_int > 0)
        {
            SPI.transfer((uint8_t)*data);
            data++;
            arg_int--;

            if (arg_int % 1024 == 0)
                CriticalLoop();
        }

        break;
    case U8X8_MSG_BYTE_INIT:
        if (u8x8->bus_clock == 0) /* issue 769 */
            u8x8->bus_clock = u8x8->display_info->sck_clock_hz;
        /* disable chipselect */
        u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);

        SPI.begin();

        break;

    case U8X8_MSG_BYTE_SET_DC:
        u8x8_gpio_SetDC(u8x8, arg_int);
        break;

    case U8X8_MSG_BYTE_START_TRANSFER:
        /* SPI mode has to be mapped to the mode of the current controller, at least Uno, Due, 101 have different SPI_MODEx values */
        internal_spi_mode = 0;
        switch (u8x8->display_info->spi_mode)
        {
        case 0:
            internal_spi_mode = SPI_MODE0;
            break;
        case 1:
            internal_spi_mode = SPI_MODE1;
            break;
        case 2:
            internal_spi_mode = SPI_MODE2;
            break;
        case 3:
            internal_spi_mode = SPI_MODE3;
            break;
        }

        SPI.beginTransaction(SPISettings(u8x8->bus_clock, MSBFIRST, internal_spi_mode));

        u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_enable_level);
        u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO, u8x8->display_info->post_chip_enable_wait_ns, NULL);
        break;

    case U8X8_MSG_BYTE_END_TRANSFER:
        u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO, u8x8->display_info->pre_chip_disable_wait_ns, NULL);
        u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);

        SPI.endTransaction();

        break;
    default:
        return 0;
    }

    return 1;
}