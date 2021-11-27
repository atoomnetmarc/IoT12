/*

Copyright 2021 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#ifndef display_h
#define display_h

#include <U8g2lib.h>

extern "C" uint8_t u8x8_byte_arduino_hw_spi_custom(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

class U8G2_PCD8544_84X48_F_4W_HW_SPI_CUSTOM : public U8G2 {
  public: U8G2_PCD8544_84X48_F_4W_HW_SPI_CUSTOM(const u8g2_cb_t *rotation, uint8_t cs, uint8_t dc, uint8_t reset = U8X8_PIN_NONE) : U8G2() {
    u8g2_Setup_pcd8544_84x48_f(&u8g2, rotation, u8x8_byte_arduino_hw_spi_custom, u8x8_gpio_and_delay_arduino);
    u8x8_SetPin_4Wire_HW_SPI(getU8x8(), cs, dc, reset);
  }
};

extern U8G2_PCD8544_84X48_F_4W_HW_SPI_CUSTOM u8g2;

void DisplaySetup(void);
void DisplayLoop(void);
void RequestDrawDisplay(void);

#endif
