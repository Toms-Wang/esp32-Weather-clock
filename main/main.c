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

#include "gui.h"
#include "key.h"
#include "WIFI.h"
#include "blufi_example_main.h"
#include "mbedtls/aes.h"

uint8_t LCD_Flash_Status = 0;
SemaphoreHandle_t BLUFI_sem = NULL;
SemaphoreHandle_t Time_sem = NULL;
SemaphoreHandle_t Weather_sem = NULL;
SemaphoreHandle_t Muxsem = NULL;

QueueHandle_t wifi_quent;
TimerHandle_t timeHandle;
TimerHandle_t weatherHandle;

uint8_t update_weather_status = 0;
uint8_t update_time_status = 0;

static const char *TAG1 = "main";

static void time_update(TimerHandle_t xTimer)
{
	(void)xTimer;
//	update_time_status = 1;
	xSemaphoreGive(Time_sem);
}

static void weather_update(TimerHandle_t xTimer)
{
	(void)xTimer;
//	update_weather_status = 1;
	xSemaphoreGive(Weather_sem);
}

void lcd_flash_task(void * parm)
{
	int flash_state = 0;
	int week_update = 0;

	while(1)
	{
		xQueueReceive(wifi_quent, &flash_state, portMAX_DELAY);

		if(flash_state == 6)
		{
			Display_CE_bc(48, 60, "wifi未连接, 请连接", WHITE, gImage_bmp320);
			flash_state = 0;
		}

		if(flash_state == 5)
		{
			Display_CE_bc(48, 60, "                   ", WHITE, gImage_bmp320);

			ESP_LOGI(TAG1, "receive queue, join in temperature update");

			http_update_time();

			gui_update_time(0, 0, gImage_bmp320);
			gui_update_weather(120, 0, gImage_bmp320);

			gui_update_week(0, 160, gImage_bmp320);

			TimerHandle_t timeHandle = xTimerCreate("time_update", 5000, pdTRUE, NULL, time_update);
			TimerHandle_t weatherHandle= xTimerCreate("weather_update", 5000 * 12 * 5, pdTRUE, NULL, weather_update);

			xTimerStart(timeHandle, 0);
			xTimerStart(weatherHandle, 0);

			while(1)
			{
				vTaskDelay(10 / portTICK_PERIOD_MS);

				if(update_weather_status)
				{
					update_weather_status = 0;
					gui_update_weather(120, 0, gImage_bmp320);
				}

				if(update_time_status)
				{
					update_time_status = 0;
					gui_update_time(0, 0, gImage_bmp320);

					struct tm* tm3 = get_tm_time();

					if(tm3->tm_hour == 0 && tm3->tm_min == 0 && tm3->tm_sec <= 5)//00:00联网更新一次（也可能是二次）时间；
					{
						gui_update_week(0, 160, gImage_bmp320);

					}

					if(!week_update)
					{
						week_update = 1;
						gui_update_week(0, 160, gImage_bmp320);
					}
				}
			}
		}
	}
}

void time_update_task(void * parm)
{
	bool time_update_first = false;
	bool week_update_first = false;
	while(1)
	{
		if(xSemaphoreTake(Time_sem, portMAX_DELAY) == pdTRUE)
		{
//			while(LCD_Flash_Status)
//			{
//				vTaskDelay(10 / portTICK_PERIOD_MS);
//			}
//			LCD_Flash_Status = 1;

			xSemaphoreTake(Muxsem, portMAX_DELAY);

			if(!time_update_first)
			{
				time_update_first = true;
				http_update_time();
				gui_update_time(0, 0, gImage_bmp320);
				gui_update_week(0, 160, gImage_bmp320);
			}
			else
			{
				gui_update_time(0, 0, gImage_bmp320);

				struct tm* tm3 = get_tm_time();

				if(tm3->tm_hour == 0 && tm3->tm_min == 0 && tm3->tm_sec <= 5)//00:00联网更新一次（也可能是二次）时间；
				{
					gui_update_week(0, 160, gImage_bmp320);

				}

				if(!week_update_first)
				{
					week_update_first = true;
					gui_update_week(0, 160, gImage_bmp320);
				}
			}

			xSemaphoreGive(Muxsem);
//			LCD_Flash_Status = 0;
		}
	}
}

void weather_update_task(void * parm)
{
	while(1)
	{
		if(xSemaphoreTake(Weather_sem, portMAX_DELAY) == pdTRUE)
		{
//			while(LCD_Flash_Status)
//			{
//				vTaskDelay(10 / portTICK_PERIOD_MS);
//			}
//			LCD_Flash_Status = 1;

			xSemaphoreTake(Muxsem, portMAX_DELAY);
			gui_update_weather(120, 0, gImage_bmp320);
			xSemaphoreGive(Muxsem);

//			LCD_Flash_Status = 0;
		}
	}
}

void app_main(void)
{
	int flash_state = 0;
    ESP_ERROR_CHECK(nvs_flash_init());

    LCD_Config_ST7789();
    spi_SD_init();
    Led_Config();
	key_config();

//	lcd_display_back(0, 0, gImage_bmp320);
    gui_update_back(gImage_bmp320);

    LED_OFF();

    BLUFI_sem = xSemaphoreCreateBinary();
    Time_sem = xSemaphoreCreateBinary();
    Weather_sem = xSemaphoreCreateBinary();

    Muxsem = xSemaphoreCreateMutex();
    xSemaphoreGive(Muxsem);

    wifi_quent = xQueueCreate(5, sizeof(int));

	Initialise_Wifi();

	xTaskCreate(time_update_task, "time_update_task", 8192, NULL, 5, NULL);
	xTaskCreate(weather_update_task, "weather_update_task", 8192, NULL, 4, NULL);

	TimerHandle_t timeHandle    = xTimerCreate("time_update", 5000, pdTRUE, NULL, time_update);
	TimerHandle_t weatherHandle = xTimerCreate("weather_update", 5000 * 12 * 5, pdTRUE, NULL, weather_update);

    while(1)
    {
    	if(xQueueReceive(wifi_quent, &flash_state, portMAX_DELAY) == pdTRUE)
    	{

			if(flash_state == 6)
			{
				Display_CE_bc(48, 60, "wifi未连接, 请连接", WHITE, gImage_bmp320);
				flash_state = 0;
			}

			if(flash_state == 5)
			{
				Display_CE_bc(48, 60, "                   ", WHITE, gImage_bmp320);

				ESP_LOGI(TAG1, "receive queue, join in temperature update");

				xSemaphoreGive(Time_sem);
				xSemaphoreGive(Weather_sem);

				xTimerStart(timeHandle, 0);
				xTimerStart(weatherHandle, 0);
			}
    	}
    }
}


