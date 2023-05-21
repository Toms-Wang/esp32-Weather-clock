#ifndef blufi_example_main_H
#define blufi_example_main_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_bt_main.h"

#include "esp_blufi_api.h"
#include "blufi_example.h"
#include "esp_blufi.h"
#include "WIFI.h"

void wifi_ble_close(void);
//void get_mac_address(char * pxaddr);
void blufi_config(void);
void BLUFI_INIT_Task(void * arg);
esp_err_t ble_deinit(void);

#endif
