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
//#include "convert.h"
#include "led.h"

#include "lwip/sockets.h"
#include "smartconfig.h"
//#include "bmp_02.h"
//#include "bmp_01.h"
//#include "bmp3.h"
//#include "bmp4_1.h"
//#include "bmp5_2.h"
//#include "bmp1.h"
#include "bmp320.h"

#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "sd_spi.h"
#include "blu_fi.h"

#include "esp_bt.h"
#include "blu_fi.h"
#include "gui.h"

#define TAG "ESP"
#define tem_num 37

uint8_t wea_status = 0;
QueueHandle_t wifi_quent;
struct tm* tm3;

static const char *TAG1 = "example1";

char week_num[7][8]= {"周日", "周一", "周二", "周三", "周四", "周五", "周六"};

char tem_bmp[tem_num][25] = {"晴", "晴夜", "多云", "晴间多云", "晴间多云夜", "大部多云",
						"大部多云夜", "阴", "阵雨", "雷阵雨", "雷阵雨伴有冰雹", "小雨",
						"中雨", "大雨", "暴雨", "大暴雨", "特大暴雨", "冻雨",
						"雨夹雪", "阵雪", "小雪", "中雪", "大雪", "暴雪",
						"浮尘", "扬沙", "沙尘暴", "强沙尘暴", "雾", "霾",
						"风", "大风", "飓风", "热带风暴", "龙卷风", "冷","热"
};

char tem_name[tem_num][15] = {	 "/bmp1.txt",  "/bmp2.txt",  "/bmp3.txt",  "/bmp4.txt",  "/bmp5.txt",  "/bmp6.txt",
								 "/bmp7.txt",  "/bmp8.txt",  "/bmp9.txt",  "/bmp10.txt", "/bmp11.txt", "/bmp12.txt",
								 "/bmp13.txt", "/bmp14.txt", "/bmp15.txt", "/bmp16.txt", "/bmp17.txt", "/bmp18.txt",
								 "/bmp19.txt", "/bmp20.txt", "/bmp21.txt", "/bmp22.txt", "/bmp23.txt", "/bmp24.txt",
								 "/bmp25.txt", "/bmp26.txt", "/bmp27.txt", "/bmp28.txt", "/bmp29.txt", "/bmp30.txt",
								 "/bmp31.txt", "/bmp32.txt", "/bmp33.txt", "/bmp34.txt", "/bmp35.txt", "/bmp36.txt", "/bmp37.txt"
};

uint8_t tim[50] = {0};
time_str_t time0;

void lcd_flash_task(void * parm)
{
	uint8_t cit[20]  = {0};
	uint8_t wea[20]  = {0};
	uint8_t tem[20]  = {0};
	uint8_t tem1[20] = {0};
	uint8_t mon1 = 0;

	int flash_state = 0;

	uint16_t p_x = 0;
	uint16_t p_y = 0;

	while(1)
	{
		//ESP_LOGI(TAG1, "task join in");
		if(wea_status == 1)
		{
			xQueueReceive(wifi_quent, &flash_state, 1000);
		}
//		xQueueReceive(wifi_quent, &flash_state, 1000);

		if(flash_state == 5 || wea_status == 0)
		{
			ESP_LOGI(TAG1, "receive queue, join in temperature update");

//			http_get_time2((char*)tim);//获取时间；
//			setTime_str((char*)tim);//更新时间；

			http_update_time();

			gui_update_time(0, 0, gImage_bmp320);

//			tm3 = get_tm_time();
//			printf("Local time2 is: %s\n", asctime(tm3));
//
//			sprintf(time0.hour, "%02d", tm3->tm_hour);
//			sprintf(time0.min, "%02d", tm3->tm_min);
//			mon1 = tm3->tm_mon + 1;
//			sprintf(time0.mon, "%02d", mon1);
//			sprintf(time0.day, "%02d", tm3->tm_mday);
//			//sprintf(time0.week, "%d", tm3->tm_wday);
//			//sprintf(time0., "%02d", tm3->tm_hour);
////			printf("mon = %d\n", (tm3->tm_mon + 1));
////			printf("mon = %s\n", time0.mon);
//
//			LCD_showStr48_bc(30, 16, time0.hour, WHITE, gImage_bmp320);
//			LCD_showStr48_bc(30, 64, time0.min, WHITE, gImage_bmp320);
//
//			Display_CE_bc(8 , 120, time0.mon, WHITE, gImage_bmp320);
//			Display_CE_bc(40 , 120, time0.day, WHITE, gImage_bmp320);
//
//			if(tm3->tm_wday >= 0 && tm3->tm_wday < 7)
//			{
//				Display_CE_bc(80 , 120, week_num[tm3->tm_wday], WHITE, gImage_bmp320);
//			}


			while(1)
			{
//				if(https_get_weather(cit, wea, tem))
//				{
//					ESP_LOGI(TAG1, "cit = %s", cit);
//					ESP_LOGI(TAG1, "wea = %s", wea);
//					ESP_LOGI(TAG1, "tem = %s", tem);
//
//					for(uint8_t i = 0; i < tem_num; i++)
//					{
//						if(strcmp((char *)tem_bmp[i], (char *)wea) == 0)
//						{
//							printf("i = %d\n", i);
////							LCD_Display(0, 	0, gImage_bmp3);
//							LCD_Display_bmp(120, 0, (char *)tem_name[i], gImage_bmp320, &p_x, &p_y);
//							printf("p_x = %d, p_y = %d\n", p_x, p_y);
//						}
//					}
//
//					if((strlen((char *)wea) /3 ) > 5)
//					{
//
//						Display_CE_bc(p_x - strlen((char*)wea) / 3 * 16 / 2, 120, (char *)wea, WHITE, gImage_bmp320);
//
//						if(strlen((char *)tem) == 1)
//						{
//							strcpy((char *)tem1, " ");
//							strcpy((char *)tem1 + 1, (char *)tem);
//						}
//						else
//						{
//							strcpy((char *)tem1, (char *)tem);
//						}
//
//						Display_CE_bc(p_x - 2 * 8, 144, (char *)tem1, WHITE, gImage_bmp320);
//						LCD_ShowChinese_C_bc(p_x, 144 , 0xA1, 0xE6, WHITE, gImage_bmp320);
//
//					}
//					else
//					{
//						Display_CE_bc(p_x - (strlen((char*)wea) / 3 * 16 + 5 * 8) / 2, 120, (char *)wea, WHITE, gImage_bmp320);
//
//						if(strlen((char *)tem) == 1)
//						{
//							strcpy((char *)tem1, " ");
//							strcpy((char *)tem1 + 1, (char *)tem);
//						}
//						else
//						{
//							strcpy((char *)tem1, (char *)tem);
//						}
//
//						Display_CE_bc(p_x + (strlen((char*)wea) / 3 * 16 + 5 * 8) / 2 - 4 * 8 , 120, (char *)tem1, WHITE, gImage_bmp320);
//						LCD_ShowChinese_C_bc(p_x + (strlen((char*)wea) / 3 * 16 + 5 * 8) / 2 - 2 * 8, 120, 0xA1, 0xE6, WHITE, gImage_bmp320);
//					}
//
//					memset((char *)tem, 0, strlen((char *)tem));
//					memset((char *)tem1, 0, strlen((char *)tem1));
//				}
//				else
//				{
//					ESP_LOGI(TAG1, "weather update fail");
//				}
				gui_update_weather(120, 0, gImage_bmp320);

				for(int j = 0; j < 15; j++)
				{
					vTaskDelay(5000 / portTICK_PERIOD_MS);
					vTaskDelay(5000 / portTICK_PERIOD_MS);
					vTaskDelay(5000 / portTICK_PERIOD_MS);
					vTaskDelay(5000 / portTICK_PERIOD_MS);

					gui_update_time(0, 0, gImage_bmp320);

//					tm3 = get_tm_time();
//					sprintf(time0.hour, "%02d", tm3->tm_hour);
//					sprintf(time0.min, "%02d", tm3->tm_min);
//					mon1 = tm3->tm_mon + 1;
//					sprintf(time0.mon, "%02d", mon1);
//					sprintf(time0.day, "%02d", tm3->tm_mday);
//
//					LCD_showStr48_bc(30, 16, time0.hour, WHITE, gImage_bmp320);
//					LCD_showStr48_bc(30, 64, time0.min, WHITE, gImage_bmp320);
//
//					Display_CE_bc(8 , 120, time0.mon, WHITE, gImage_bmp320);
//					Display_CE_bc(40 , 120, time0.day, WHITE, gImage_bmp320);
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

    LCD_Display(0, 	0, gImage_bmp320);

    char str[15] = {0};
    char filename[50] = {0};
    uint16_t z_x = 0;
    uint16_t z_y = 0;

    LCD_showChar48_bc(30, 16, '1', WHITE, gImage_bmp320);
    LCD_showChar48_bc(54, 16, '5', WHITE, gImage_bmp320);
    LCD_showChar48_bc(30, 64, '4', WHITE, gImage_bmp320);
    LCD_showChar48_bc(54, 64, '6', WHITE, gImage_bmp320);

    //LCD_showStr48_bc(54, 180, "16", WHITE, gImage_bmp320);

    //Display_CE_bc(44 , 136, (char *)"周二", WHITE, gImage_bmp320);
    Display_CE_bc(8 , 120, (char *)"03月20日 周一", WHITE, gImage_bmp320);

//	for(int i = 0; i < 37; i++)
//	{
//		sprintf(str, "%d", (i + 1));
//		strcpy(filename, "/bmp");
//		strcpy(filename + 4, str);
//		strcpy(filename + 4 + strlen(str), ".txt");
//		//LCD_Display(0, 0, gImage_bmp3);//背景；
//		LCD_Display_bmp(120, 0, (char *)filename, gImage_bmp320, &z_x, &z_y);
//
//		printf("p_x = %d, p_y = %d\n", z_x, z_y);
//		vTaskDelay(300 / portTICK_PERIOD_MS);
//	}

    LCD_Display_bmp(120, 0, (char *)"/bmp2.txt", gImage_bmp320, &z_x, &z_y);

    Led_Config();
    LED_OFF();

    wifi_quent = xQueueCreate(5, sizeof(int));

    //wifi_blufi_config();

    wifi_config_t myconfig = {0};
	esp_wifi_get_config(ESP_IF_WIFI_STA, &myconfig);//获取过配网信息。

	if(strlen((char *)myconfig.sta.ssid) > 0)
	{
		ESP_LOGI(TAG, "already set, SSID is: %s, start connect", myconfig.sta.ssid);
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
