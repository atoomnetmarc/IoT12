/*

Copyright 2021 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#ifndef config_h
#define config_h

extern const char *ssid;
extern const char *password;

extern const char *mqtt_topic_prefix;
extern const char *mqtt_server;
extern const char *mqtt_username;
extern const char *mqtt_password;

extern const char *otapassword;
extern const char *otahostname;

#define PIN_LCD_LED 32
#define PIN_LCD_DATACMD 4
#define PIN_LCD_CE 5
#define PIN_LCD_CLK 18
#define PIN_LCD_RST 2
#define PIN_LCD_DIN 23

#define PIN_HEAT_INT 35

#define PIN_ROT_A 16
#define PIN_ROT_B 17
#define PIN_ROT_SW 33

#define PIN_I2S_CLK 25
#define PIN_I2S_WS 26
#define PIN_I2S_DATA 27

#endif