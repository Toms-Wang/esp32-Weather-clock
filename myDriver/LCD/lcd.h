#ifndef LCD_H_
#define LCD_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define WHITE            0xFFFF
#define BLACK            0x0000
#define BLUE             0x001F
#define BRED             0XF81F
#define GRED             0XFFE0
#define GBLUE            0X07FF
#define RED              0xF800
#define MAGENTA          0xF81F
#define GREEN            0x07E0
#define CYAN             0x7FFF
#define YELLOW           0xFFE0
#define BROWN            0XBC40 //棕色
#define BRRED            0XFC07 //棕红色
#define GRAY             0X8430 //灰色
//GUI颜色

#define DARKBLUE         0X01CF //深蓝色
#define LIGHTBLUE        0X7D7C //浅蓝色
#define GRAYBLUE         0X5458 //灰蓝色
//以上三色为PANEL的颜色

#define LIGHTGREEN       0X841F //浅绿色
#define LGRAY            0XC618 //浅灰色(PANNEL),窗体背景色

#define LGRAYBLUE        0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE           0X2B12 //浅棕蓝色(选择条目的反色)

#define LCD_W 128
#define LCD_H 128

#ifdef CONFIG_IDF_TARGET_ESP32
#define LCD_HOST    HSPI_HOST

#define PIN_NUM_MISO 25
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  19

#define PIN_NUM_CS   22

#define PIN_NUM_DC   21
#define PIN_NUM_RST  18
#define PIN_NUM_BCKL 15

#elif defined CONFIG_IDF_TARGET_ESP32S2
#define LCD_HOST    SPI2_HOST

#define PIN_NUM_MISO 37
#define PIN_NUM_MOSI 35
#define PIN_NUM_CLK  36
#define PIN_NUM_CS   34

#define PIN_NUM_DC   4
#define PIN_NUM_RST  5
#define PIN_NUM_BCKL 6
#elif defined CONFIG_IDF_TARGET_ESP32C3
#define LCD_HOST    SPI2_HOST

#define PIN_NUM_MISO 2
#define PIN_NUM_MOSI 7
#define PIN_NUM_CLK  6
#define PIN_NUM_CS   10

#define PIN_NUM_DC   9
#define PIN_NUM_RST  4
#define PIN_NUM_BCKL 5
#endif

void LCD_clear(uint16_t color);
void LCD_Fill(uint16_t color);
void LCD_Config(void);
void LCD_showChar(uint16_t x, uint16_t y, uint8_t chr, uint16_t color);
void LCD_showString(uint16_t x, uint16_t y, char *p, uint16_t color);
void LCD_ShowIntNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint16_t color);

#endif
