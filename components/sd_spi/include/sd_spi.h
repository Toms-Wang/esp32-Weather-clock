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

#define SD_HOST    SPI3_HOST
#define MOUNT_POINT "/sdcard"

#define PIN_SD_MISO 40
#define PIN_SD_MOSI 42
#define PIN_SD_CLK  41

#define PIN_SD_CS   8

void SD_GPIO_Init(void);
void spi_SD_init(void);

void SD_GPIO_Init_14(void);

#endif

