/* HTTPS GET Example using plain mbedTLS sockets
 *
 * Contacts the howsmyssl.com API via TLS v1.2 and reads a JSON
 * response.
 *
 * Adapted from the ssl_client1 example in mbedtls.
 *
 * SPDX-FileCopyrightText: 2006-2016 ARM Limited, All Rights Reserved
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * SPDX-FileContributor: 2015-2022 Espressif Systems (Shanghai) CO LTD
 */
#include "sdkconfig.h"
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "cJSON.h"
#include "lcd.h"
#include "weather.h"
#include "tim_data.h"

#include "led.h"

#include "lwip/sockets.h"
#include "smartconfig.h"
#include "bmp320.h"

#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "sd_spi.h"
#include "blu_fi.h"

#include "esp_bt.h"
#include "blu_fi.h"
#include "gui.h"

uint8_t wea_status = 0;
QueueHandle_t wifi_quent;

static const char *TAG1 = "example1";

void lcd_flash_task(void * parm)
{
	int flash_state = 0;

	while(1)
	{
		if(wea_status == 1)
		{
			xQueueReceive(wifi_quent, &flash_state, 1000);
		}

		if(flash_state == 5 || wea_status == 0)
		{
			ESP_LOGI(TAG1, "receive queue, join in temperature update");

			http_update_time();

			gui_update_time(0, 0, gImage_bmp320);

			while(1)
			{

				gui_update_weather(120, 0, gImage_bmp320);

				for(int j = 0; j < 15; j++)
				{
					vTaskDelay(5000 / portTICK_PERIOD_MS);
					vTaskDelay(5000 / portTICK_PERIOD_MS);
					vTaskDelay(5000 / portTICK_PERIOD_MS);
					vTaskDelay(5000 / portTICK_PERIOD_MS);

					gui_update_time(0, 0, gImage_bmp320);
				}
			}
		}
	}
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    LCD_Config_ST7789();
    spi_SD_init();
    Led_Config();

    gui_update_back(gImage_bmp320);
//    LCD_Display(0, 	0, gImage_bmp320);

    LED_OFF();

    wifi_quent = xQueueCreate(5, sizeof(int));

    wifi_config_t myconfig = {0};
	esp_wifi_get_config(ESP_IF_WIFI_STA, &myconfig);//获取过配网信息。

	if(strlen((char *)myconfig.sta.ssid) > 0)
	{
		ESP_LOGI(TAG1, "already set, SSID is: %s, start connect", myconfig.sta.ssid);
		esp_wifi_connect();
	}
	else
	{
		//wifi_blufi_config();
		initialise_wifi();
		wea_status = 1;
	}
	xTaskCreate(lcd_flash_task, "lcd_flash_task", 4096, NULL, 3, NULL);
    //initialise_wifi();

    while(1)
    {
    	vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
