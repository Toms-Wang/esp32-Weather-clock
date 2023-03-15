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
#include "time.h"
#include "convert.h"
#include "led.h"

#include "lwip/sockets.h"
#include "smartconfig.h"
#include "bmp_02.h"
#include "bmp_01.h"
#include "bmp3.h"
#include "bmp4_1.h"
#include "bmp5_2.h"

#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "sd_spi.h"

#define TAG "ESP"

extern QueueHandle_t wifi_quent;

static const char *TAG1 = "example1";


void lcd_flash_task(void * parm)
{
	uint8_t cit[20] = {0};
	uint8_t wea[20] = {0};
	uint8_t tem[20] = {0};
	uint8_t tem1[20] = {0};
	int flash_state = 0;
	while(1)
	{
		//ESP_LOGI(TAG1, "task join in");
		xQueueReceive(wifi_quent, &flash_state, 100000);

		if(flash_state == 5)
		{
			ESP_LOGI(TAG1, "receive queue, join in temperature update");
			while(1)
			{
				if(https_get_weather(cit, wea, tem))
				{
					ESP_LOGI(TAG1, "cit = %s", cit);
					ESP_LOGI(TAG1, "wea = %s", wea);
					ESP_LOGI(TAG1, "tem = %s", tem);

					Display_CE(16, 28, (char *)cit, WHITE);
					Display_CE(66, 55, (char *)wea, WHITE);

					if(strlen((char *)tem) == 1)
					{
						strcpy((char *)tem1, " ");
						strcpy((char *)tem1 + 1, (char *)tem);
						strcpy((char *)tem1 + 2, "度");
						Display_CE(86, 55, (char *)tem1, WHITE);
					}
					else
					{
						strcpy((char *)tem1, (char *)tem);
						strcpy((char *)tem1 + 2, "度");
						Display_CE(86, 55, (char *)tem1, WHITE);
					}

					memset((char *)tem, 0, strlen((char *)tem));
					memset((char *)tem1, 0, strlen((char *)tem1));
				}
				else
				{
					ESP_LOGI(TAG1, "weather update fail");
				}

				//Display_CE(0, 80, test_chinese, WHITE);
				//Display_CE(0, 100, "℃", WHITE);

				for(int j = 0; j < 15; j++)
				{
					vTaskDelay(5000 / portTICK_PERIOD_MS);
					vTaskDelay(5000 / portTICK_PERIOD_MS);
					vTaskDelay(5000 / portTICK_PERIOD_MS);
					vTaskDelay(5000 / portTICK_PERIOD_MS);
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

    LCD_Config();
    LCD_Fill(WHITE);
    LCD_Display(0, 	0, gImage_bmp3);
    //LCD_Display(60, 7, gImage_bmp5_2);
    //LCD_Display_Icon(70, 7, gImage_bmp_01, gImage_bmp3);
    LCD_Display_Icon(60, 7, gImage_bmp5_2, gImage_bmp3);

    spi_SD_init();

    Led_Config();
    LED_OFF();
    initialise_wifi();
    wifi_quent = xQueueCreate(5, sizeof(int));
    xTaskCreate(lcd_flash_task, "lcd_flash_task", 4096, NULL, 3, NULL);

    while(1)
    {
    	vTaskDelay(100 / portTICK_PERIOD_MS);

    }
}
