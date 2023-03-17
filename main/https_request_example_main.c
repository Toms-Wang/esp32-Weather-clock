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
#include "bmp1.h"

#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "sd_spi.h"

#define TAG "ESP"

extern QueueHandle_t wifi_quent;

static const char *TAG1 = "example1";

#define tem_num 37

char tem_bmp[tem_num][25] = {"晴", "晴夜", "多云", "晴间多云", "晴间多云夜", "大部多云",
						"大部多云夜", "阴", "阵雨", "雷阵雨", "雷阵雨伴有冰雹", "小雨",
						"中雨", "大雨", "暴雨", "大暴雨", "特大暴雨", "冻雨",
						"雨夹雪", "阵雪", "小雪", "中雪", "大雪", "暴雪",
						"浮尘", "扬沙", "沙尘暴", "强沙尘暴", "雾", "霾",
						"风", "大风", "飓风", "热带风暴", "龙卷风", "冷","热"



};

char tem_name[tem_num][15] = {"/bmp1.txt",  "/bmp2.txt",  "/bmp3.txt",  "/bmp4.txt",  "/bmp5.txt",  "/bmp6.txt",
						 "/bmp7.txt",  "/bmp8.txt",  "/bmp9.txt",  "/bmp10.txt", "/bmp11.txt", "/bmp12.txt",
						 "/bmp13.txt", "/bmp14.txt", "/bmp15.txt", "/bmp16.txt", "/bmp17.txt", "/bmp18.txt",
						 "/bmp19.txt", "/bmp20.txt", "/bmp21.txt", "/bmp22.txt", "/bmp23.txt", "/bmp24.txt",
						 "/bmp25.txt", "/bmp26.txt", "/bmp27.txt", "/bmp28.txt", "/bmp29.txt", "/bmp30.txt",
						 "/bmp31.txt", "/bmp32.txt", "/bmp33.txt", "/bmp34.txt", "/bmp35.txt", "/bmp36.txt", "/bmp37.txt"
};

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

					for(uint8_t i = 0; i < tem_num; i++)
					{
						if(strcmp((char *)tem_bmp[i], (char *)wea) == 0)
						{
							printf("i = %d\n", i);
							//LCD_Display(0, 	0, gImage_bmp3);
							LCD_Display_bmp(4, 4, (char *)tem_name[i], gImage_bmp3);
						}
					}

					Display_CE(16, 28, (char *)cit, WHITE);
					Display_CE(82 - strlen((char*)wea) / 3 * 16, 55, (char *)wea, WHITE);

					if(strlen((char *)tem) == 1)
					{
						strcpy((char *)tem1, " ");
						strcpy((char *)tem1 + 1, (char *)tem);
						//strcpy((char *)tem1 + 2, "度");
						Display_CE(86, 55, (char *)tem1, WHITE);
						LCD_ShowChinese_C(102, 55, 0xA1, 0xE6, WHITE);
					}
					else
					{
						strcpy((char *)tem1, (char *)tem);
						//strcpy((char *)tem1 + 2, "度");
						Display_CE(86, 55, (char *)tem1, WHITE);
						LCD_ShowChinese_C(102, 55, 0xA1, 0xE6, WHITE);
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
    spi_SD_init();

    LCD_Fill(WHITE);
    LCD_Display(0, 	0, gImage_bmp3);

    //LCD_Display_Icon(60, 7, gImage_bmp5_2, gImage_bmp3);gImage_bmp1

    //LCD_Display_Icon(0, 0, gImage_bmp1, gImage_bmp3);

    //LCD_Display_52(60, 7, gImage_bmp3);

    char str[15] = {0};
    char filename[50] = {0};

	for(int i = 0; i < 37; i++)
	{
		sprintf(str, "%d", (i + 1));
		strcpy(filename, "/bmp");
		strcpy(filename + 4, str);
		strcpy(filename + 4 + strlen(str), ".txt");
		//LCD_Display(0, 	0, gImage_bmp3);
		LCD_Display_bmp(4, 4, (char *)filename, gImage_bmp3);
		vTaskDelay(300 / portTICK_PERIOD_MS);
	}

    LCD_Display_bmp(4, 4, (char *)"/bmp2.txt", gImage_bmp3);

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
