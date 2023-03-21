#ifndef sd_spi_H
#define sd_spi_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"

//SDMMC_FREQ_DEFAULT
#define SD_HOST    VSPI_HOST
#define MOUNT_POINT "/sdcard"

//#define PIN_SD_MISO 25
//#define PIN_SD_MOSI 26
//#define PIN_SD_CLK  27
//
////#define PIN_SD_CS   14
//#define PIN_SD_CS   23
////#define PIN_SD_CS   32

#define PIN_SD_CS   5
#define PIN_SD_CLK  18
#define PIN_SD_MISO 19
#define PIN_SD_MOSI 23

void SD_GPIO_Init(void);
void spi_SD_init(void);

void SD_GPIO_Init_14(void);

#endif

