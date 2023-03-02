#ifndef time_H
#define time_H

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

#include "esp_tls.h"
#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
#include "esp_crt_bundle.h"
#endif

/* Constants that aren't configurable in menuconfig */
#define WEB_TIME_SERVER "www.howsmyssl.com"
#define WEB_TIME_PORT "443"
//#define WEB_URL "https://api.seniverse.com/v3/weather/now.json?key=SfAutZrLlsHTBVSfN&location=shanghai&language=zh-Hans&unit=c"
#define WEB_TIME_URL "http://quan.suning.com/getSysTime.do"


#define SERVER_URL_TIME_MAX_SZ 256

uint8_t https_get_time(uint8_t * pxtime);

#endif
