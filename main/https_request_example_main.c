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

static const char *TAG1 = "example1";


void app_main(void)
{
	uint8_t cit[20] = {0};
	uint8_t wea[20] = {0};
	uint8_t tem[20] = {0};
    ESP_ERROR_CHECK( nvs_flash_init() );
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    LCD_Config();
    LCD_Fill(WHITE);

    ESP_ERROR_CHECK(example_connect());

    while(1)
    {
    	https_get_weather(cit, wea, tem);
    	ESP_LOGI(TAG1, "cit = %s", cit);
    	ESP_LOGI(TAG1, "wea = %s", wea);
    	ESP_LOGI(TAG1, "tem = %s", tem);
    	vTaskDelay(5000 / portTICK_PERIOD_MS);
    	vTaskDelay(5000 / portTICK_PERIOD_MS);
    	vTaskDelay(5000 / portTICK_PERIOD_MS);
    	vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
