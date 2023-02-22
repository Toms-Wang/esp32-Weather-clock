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
#include "weather.h"


static const char *TAG = "example";

static const char HOWSMYSSL_REQUEST[] = "GET " WEB_URL " HTTP/1.1\r\n"
                             "Host: "WEB_SERVER"\r\n"
                             "User-Agent: esp-idf/1.0 esp32\r\n"
                             "\r\n";

/* Root cert for howsmyssl.com, taken from server_root_cert.pem

   The PEM file was extracted from the output of this command:
   openssl s_client -showcerts -connect www.howsmyssl.com:443 </dev/null

   The CA root cert is the last cert given in the chain of certs.

   To embed it in the app binary, the PEM file is named
   in the component.mk COMPONENT_EMBED_TXTFILES variable.
*/
extern const uint8_t server_root_cert_pem_start[] asm("_binary_server_root_cert_pem_start");
extern const uint8_t server_root_cert_pem_end[]   asm("_binary_server_root_cert_pem_end");

static uint8_t https_get_request(esp_tls_cfg_t cfg, char * pxcit, char * pxwea, char * pxtem)
{
	uint8_t Get_weather_status = 0;
    char buf[512];
    int ret, len;
//    char cit[20] = {0};
//    char wea[20] = {0};
//    char tem[20] = {0};

    struct esp_tls *tls = esp_tls_conn_http_new(WEB_URL, &cfg);

    if (tls != NULL)
    {
        ESP_LOGI(TAG, "Connection established..."); //连接建立服务
    }
    else
    {
        ESP_LOGE(TAG, "Connection failed..."); //连接失败

        goto exit;
    }

    size_t written_bytes = 0;
    do
    {
        ret = esp_tls_conn_write(tls,
        		HOWSMYSSL_REQUEST + written_bytes,
                                 sizeof(HOWSMYSSL_REQUEST) - written_bytes);
        if (ret >= 0)
        {
            ESP_LOGI(TAG, "%d bytes written", ret);
            written_bytes += ret;
        }
        else if (ret != ESP_TLS_ERR_SSL_WANT_READ && ret != ESP_TLS_ERR_SSL_WANT_WRITE)
        {
            ESP_LOGE(TAG, "esp_tls_conn_write  returned: [0x%02X](%s)", ret, esp_err_to_name(ret));
            goto exit;
        }
    } while (written_bytes < sizeof(HOWSMYSSL_REQUEST));

    ESP_LOGI(TAG, "Reading HTTP response...");

    do
    {
        len = sizeof(buf) - 1;
        bzero(buf, sizeof(buf));
        ret = esp_tls_conn_read(tls, (char *)buf, len);

        if (ret == ESP_TLS_ERR_SSL_WANT_WRITE || ret == ESP_TLS_ERR_SSL_WANT_READ)
        {
            continue;
        }

        if (ret < 0)
        {
            ESP_LOGE(TAG, "esp_tls_conn_read  returned [-0x%02X](%s)", -ret, esp_err_to_name(ret));
            break;
        }

        if (ret == 0)
        {
            ESP_LOGI(TAG, "connection closed");
            break;
        }

        len = ret;
        ESP_LOGD(TAG, "%d bytes read", len);
        /* 当读取响应时，直接将其打印到标准输出 */
        for (int i = 0; i < len; i++)
        {
            putchar(buf[i]);
        }
        putchar('\n'); // JSON输出的末尾没有换行符

        char *str_1 = strchr(buf, '['); //查找'{'
        if (str_1 != NULL)
        {
            char *str_2 = strchr(buf, ']'); //查找'}'
            ESP_LOGI(TAG, "str_1 = %d  str_2 = %d    %d", (int)str_1, (int)str_2, (int)(str_2 - str_1));
            char dest[300] = {""};
            strncpy(dest, str_1 - 11, str_2 - str_1 + 11 + 1 + 1); //将{ }里的数据拷贝到dest字符串里

            ESP_LOGI(TAG, "%s", dest);

            cJSON *pJsonRoot = cJSON_Parse(dest);
            //如果是否json格式数据
            if (pJsonRoot != NULL)
            {
            	Get_weather_status = 1;
                cJSON *json_results = cJSON_GetObjectItem(pJsonRoot, "results");
                if(json_results)
                {
                	int size = cJSON_GetArraySize(json_results);
                	ESP_LOGI(TAG, "results.size = %d", size);

                	cJSON *re_child = cJSON_GetArrayItem(json_results, 0);
                	if(re_child)
                	{
                		cJSON * json_location    = cJSON_GetObjectItem(re_child, "location");
                		if(json_location)
                		{
                			cJSON * json_name = cJSON_GetObjectItem(json_location, "name");
                			ESP_LOGI(TAG, "%s = %s", json_name -> string, json_name -> valuestring);
//                			memset(cit, 0, strlen(cit));
//                			strcpy(cit, json_name -> valuestring);

                			memset(pxcit, 0, strlen(pxcit));
                			strcpy(pxcit, json_name -> valuestring);
                		}

                		cJSON * json_now  = cJSON_GetObjectItem(re_child, "now");
                		if(json_now)
						{
							cJSON * json_text = cJSON_GetObjectItem(json_now, "text");
							ESP_LOGI(TAG, "%s = %s", json_text -> string, json_text -> valuestring);
							memset(pxwea, 0, strlen(pxwea));
							strcpy(pxwea, json_text -> valuestring);

							cJSON * json_temerature = cJSON_GetObjectItem(json_now, "temperature");
							ESP_LOGI(TAG, "%s = %s", json_temerature -> string, json_temerature -> valuestring);
							memset(pxtem, 0, strlen(pxtem));
							strcpy(pxtem, json_temerature -> valuestring);
						}
                		cJSON * json_last_update = cJSON_GetObjectItem(re_child, "last_update");
                		ESP_LOGI(TAG, "%s = %s", json_last_update -> string, json_last_update -> valuestring);
                	}
                }

                ESP_LOGI(TAG, "城市：%s", pxcit);
                //LCD_showString(32, 64, , RED);
                //LCD_showString(32, 64, wea, RED);
                ESP_LOGI(TAG, "天气：%s", pxwea);
                ESP_LOGI(TAG, "温度：%s", pxtem);

                LCD_showString(32, 84, "tem:", RED);
                LCD_showString(64, 84, pxtem, RED);

            }
        }
    } while (1);

exit:
    esp_tls_conn_delete(tls);
    for (int countdown = 10; countdown >= 0; countdown--)
    {
        ESP_LOGI(TAG, "%d...", countdown);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    return Get_weather_status;
}

//static void https_get_request_using_crt_bundle(uint8_t * pxcit1, uint8_t * pxwea1, uint8_t * pxtem1)
//{
//    ESP_LOGI(TAG, "https_request using crt bundle");
//    esp_tls_cfg_t cfg =
//    {
//        .crt_bundle_attach = esp_crt_bundle_attach,
//    };
//    https_get_request(cfg);
//}

uint8_t https_get_weather(uint8_t * pxcit, uint8_t * pxwea, uint8_t * pxtem)
{
	ESP_LOGI(TAG, "https_request using crt bundle");
	esp_tls_cfg_t cfg =
	{
		.crt_bundle_attach = esp_crt_bundle_attach,
	};

	return https_get_request(cfg, (char*)pxcit, (char*)pxwea, (char*)pxtem);
}

//static void https_get_request_using_cacert_buf(void)
//{
//    ESP_LOGI(TAG, "https_request using cacert_buf");
//    esp_tls_cfg_t cfg = {
//        .cacert_buf = (const unsigned char *) server_root_cert_pem_start,
//        .cacert_bytes = server_root_cert_pem_end - server_root_cert_pem_start,
//    };
//    https_get_request(cfg);
//}
//
//static void https_get_request_using_global_ca_store(void)
//{
//    esp_err_t esp_ret = ESP_FAIL;
//    ESP_LOGI(TAG, "https_request using global ca_store");
//    esp_ret = esp_tls_set_global_ca_store(server_root_cert_pem_start, server_root_cert_pem_end - server_root_cert_pem_start);
//    if (esp_ret != ESP_OK) {
//        ESP_LOGE(TAG, "Error in setting the global ca store: [%02X] (%s),could not complete the https_request using global_ca_store", esp_ret, esp_err_to_name(esp_ret));
//        return;
//    }
//    esp_tls_cfg_t cfg = {
//        .use_global_ca_store = true,
//    };
//    https_get_request(cfg);
//    esp_tls_free_global_ca_store();
//}
//
//static void https_request_task(void *pvparameters)
//{
//    ESP_LOGI(TAG, "Start https_request example");
//
//#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
//    https_get_request_using_crt_bundle();
//#endif
//    printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());
//    https_get_request_using_cacert_buf();
//    https_get_request_using_global_ca_store();
//    ESP_LOGI(TAG, "Finish https_request example");
//    vTaskDelete(NULL);
//}

