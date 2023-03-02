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

static const char *TAG1 = "example1";

char test_chinese[] = "中文编码测试";
char *test1;

//城
uint8_t ch_1[32] = {32,40,32,36,32,32,39,254,36,32,252,32,36,36,39,164,
		36,164,36,168,36,168,60,144,230,146,73,42,8,70,16,130};
//市
uint8_t ch_2[32] = { 2,0,1,0,0,0,127,252,1,0,1,0,1,0,63,248,
		33,8,33,8,33,8,33,8,33,40,33,16,1,0,1,0};
//上
uint8_t ch_3[32] = {2,0,2,0,2,0,2,0,2,0,2,0,3,248,2,0,
		2,0,2,0,2,0,2,0,2,0,2,0,255,254,0,0};
//海
uint8_t ch_4[32] = {1,0,33,0,17,252,18,0,133,248,65,8,73,72,9,40,
		23,254,17,8,226,72,34,40,35,252,32,8,32,80,0,32};

//温
uint8_t ch_5[32] = { 0,0,35,248,18,8,18,8,131,248,66,8,66,8,19,248,
		16,0,39,252,228,164,36,164,36,164,36,164,47,254,0,0};

//度
uint8_t ch_6[32] = {1,0,0,128,63,254,34,32,34,32,63,252,34,32,34,32,
		35,224,32,0,47,240,36,16,66,32,65,192,134,48,56,14};


void app_main(void)
{
	uint8_t cit[20] = {0};
	uint8_t wea[20] = {0};
	uint8_t tem[20] = {0};

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    LCD_Config();
    LCD_Fill(WHITE);

    ESP_ERROR_CHECK(example_connect());

    while(1)
    {
    	if(https_get_weather(cit, wea, tem))
    	{
    		ESP_LOGI(TAG1, "cit = %s", cit);
			ESP_LOGI(TAG1, "wea = %s", wea);
			ESP_LOGI(TAG1, "tem = %s", tem);

			Show_Dis_Chinese(28, 40, ch_1, RED);//城市；
			Show_Dis_Chinese(44, 40, ch_2, RED);
			LCD_showString(60, 40, (char*)":", RED);
			Show_Dis_Chinese(68, 40, ch_3, RED);//上海；
			Show_Dis_Chinese(84, 40, ch_4, RED);

			Show_Dis_Chinese(28, 72, ch_5, RED);//温度；
			Show_Dis_Chinese(44, 72, ch_6, RED);
			LCD_showString(60, 72, (char*)":", RED);
			if(strlen((char *)tem) == 1)
			{
				LCD_showString(68, 72, " ", RED);
				LCD_showString(76, 72, (char*)tem, RED);
				Show_Dis_Chinese(84, 72, ch_6, RED);//度；
			}
			else
			{
				LCD_showString(68, 72, (char*)tem, RED);
				Show_Dis_Chinese(84, 72, ch_6, RED);
			}

			memset((char *)tem, 0, strlen((char *)tem));

    	}
    	else
    	{
    		ESP_LOGI(TAG1, "weather update fail");
    	}

//    	utf82gbk(&test1, test_chinese, strlen(test_chinese));

    	vTaskDelay(5000 / portTICK_PERIOD_MS);
    	vTaskDelay(5000 / portTICK_PERIOD_MS);
    	vTaskDelay(5000 / portTICK_PERIOD_MS);
    	vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
