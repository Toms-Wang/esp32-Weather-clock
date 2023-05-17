#ifndef tim_data_H
#define tim_data_H

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
//#include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "cJSON.h"
//#include "bsp_wifi_station.h"
#include "esp_http_client.h"

typedef struct time_str
{
	char year[5];
	char mon[5];
	char day[5];
	char hour[5];
	char min[5];
	char sec[5];
	char week[5];
}time_str_t;

uint8_t http_get_time(char * pxtim);
uint8_t http_get_time2(char * pxtim);
struct tm * get_tm_time(void);
void setTime_str(char * pxtim);

#endif
