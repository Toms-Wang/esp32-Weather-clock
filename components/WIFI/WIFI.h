#ifndef WIFI_H
#define WIFI_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_mac.h"
#include "esp_wifi.h"
//#include "esp_blufi_api.h"
//#include "esp_blufi.h"
#include "esp_log.h"

void Initialise_Wifi(void);
void update_mac_address(char *mac);
void get_mac_address(void);
void read_mac_address(char *mac);

void initialise_wifi(void);
void reset_wifi(void);

int softap_get_current_connection_number(void);

#endif
