#include "WIFI.h"
#include "esp_blufi_api.h"
#include "esp_blufi.h"

#define EXAMPLE_WIFI_CONNECTION_MAXIMUM_RETRY  2
#define EXAMPLE_INVALID_REASON                 255
#define EXAMPLE_INVALID_RSSI                   -128

static const char *TAG = "WIFI";

wifi_config_t sta_config;
wifi_config_t sta_get_config;

uint8_t blufi_state = 0;

extern SemaphoreHandle_t BLUFI_sem;
extern QueueHandle_t wifi_quent;

static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;
static uint8_t example_wifi_retry = 0;

bool gl_sta_connected = false;
uint8_t gl_sta_bssid[6];
uint8_t gl_sta_ssid[32];
int gl_sta_ssid_len;
wifi_sta_list_t gl_sta_list;
bool gl_sta_is_connecting = false;
esp_blufi_extra_info_t gl_sta_conn_info;


extern bool ble_is_connected;
TimerHandle_t   WIFI_Handle;
TimerHandle_t   Blufi_Handle;

char mac_address[13] = {0};
extern void blufi_config(void);

extern void ble_close(void);

static void ip_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data);

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data);

static void wifi_timer(TimerHandle_t xTimer)
{
    (void)xTimer;
    printf("enter wifi_timer\n");
    if(gl_sta_connected == false)
    {
        printf("wifi connect timeout, enter Blufi_mode\n");

        if(blufi_state == 0)
        {
            blufi_state = 1;
            xTimerReset(Blufi_Handle, 0);
            blufi_config();
//            xSemaphoreGive(BLUFI_sem);
        }
    }
}

static void Blufi_timer(TimerHandle_t xTimer)
{
    (void)xTimer;
    printf("enter Blufi_timer\n");
}


void update_mac_address(char *mac)
{
	if(mac == NULL || strlen(mac) != 12)
	{
		printf("mac update error!\n");
		return;
	}
	else
	{
		strncpy(mac_address, mac, 12);
	}
}

void read_mac_address(char *mac)
{
	if(mac == NULL)
	{
		printf("mac read error!\n");
		return;
	}
	else
	{
		strncpy(mac ,mac_address, 12);
	}
}

void get_mac_address(void)
{
    const uint8_t mac_len = 6;
    char mac[13] = {0};
    uint8_t mac_addr[6] = {0};
    esp_read_mac(mac_addr, ESP_MAC_WIFI_STA);

    for(int i = 0; i < mac_len; i++)
    {
        sprintf(mac + i * 2, "%02x", mac_addr[i]);
    }
    update_mac_address(mac);
    ESP_LOGI(TAG, "get mac address:%s", mac);
}

static void example_record_wifi_conn_info(int rssi, uint8_t reason)
{
    memset(&gl_sta_conn_info, 0, sizeof(esp_blufi_extra_info_t));
    if (gl_sta_is_connecting) {
        gl_sta_conn_info.sta_max_conn_retry_set = true;
        gl_sta_conn_info.sta_max_conn_retry = EXAMPLE_WIFI_CONNECTION_MAXIMUM_RETRY;
    } else {
        gl_sta_conn_info.sta_conn_rssi_set = true;
        gl_sta_conn_info.sta_conn_rssi = rssi;
        gl_sta_conn_info.sta_conn_end_reason_set = true;
        gl_sta_conn_info.sta_conn_end_reason = reason;
    }
}

static void example_wifi_connect(void)
{
    example_wifi_retry = 0;
    gl_sta_is_connecting = (esp_wifi_connect() == ESP_OK);
    example_record_wifi_conn_info(EXAMPLE_INVALID_RSSI, EXAMPLE_INVALID_REASON);
}


static bool example_wifi_reconnect(void)
{
    bool ret;
    if (gl_sta_is_connecting && example_wifi_retry++ < EXAMPLE_WIFI_CONNECTION_MAXIMUM_RETRY) {
    	ESP_LOGI(TAG, "BLUFI WiFi starts reconnection\n");
        gl_sta_is_connecting = (esp_wifi_connect() == ESP_OK);
        example_record_wifi_conn_info(EXAMPLE_INVALID_RSSI, EXAMPLE_INVALID_REASON);
        ret = true;
    } else {
        ret = false;
    }
    return ret;
}

int softap_get_current_connection_number(void)
{
    esp_err_t ret;
    ret = esp_wifi_ap_get_sta_list(&gl_sta_list);
    if (ret == ESP_OK)
    {
        return gl_sta_list.num;
    }

    return 0;
}

static void ip_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    wifi_mode_t mode;

    switch (event_id)
    {
        case IP_EVENT_STA_GOT_IP:
        {
            esp_blufi_extra_info_t info;

            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
            esp_wifi_get_mode(&mode);

            memset(&info, 0, sizeof(esp_blufi_extra_info_t));
            memcpy(info.sta_bssid, gl_sta_bssid, 6);
            info.sta_bssid_set = true;
            info.sta_ssid = gl_sta_ssid;
            info.sta_ssid_len = gl_sta_ssid_len;
            if (ble_is_connected == true)
            {
                esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_SUCCESS, softap_get_current_connection_number(), &info);
            }
            else
            {
                ESP_LOGI(TAG, "BLUFI BLE is not connected yet\n");
            }

//            LED_OFF();
            xTimerStop(WIFI_Handle, 0);
            xTimerStop(Blufi_Handle, 0);


            int send2 = 5;
            xQueueSend(wifi_quent, &send2, 10000);
//            set_task_status(EV_SNTP_TASK);

            break;
        }
        default:
            break;
    }
    return;
}


static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    wifi_event_sta_connected_t *event;
	wifi_event_sta_disconnected_t *disconnected_event;

//    wifi_mode_t mode;

    switch (event_id)
    {
    case WIFI_EVENT_STA_START:

        esp_wifi_get_config(WIFI_IF_STA, &sta_get_config);
        if(strlen((char *)sta_get_config.sta.ssid) > 0)//if wifi set ssid, password
        {
            ESP_LOGI(TAG, "wifi start connect to AP");
            xTimerReset(WIFI_Handle, 0);
            esp_wifi_connect();
        }
        else//not set ssid
        {
            ESP_LOGI(TAG, "enter blufi mode");
            printf("state = %d\n", blufi_state);
            if(blufi_state == 0)
            {
                blufi_state = 1;
              blufi_config();
//                LED_ON();
//                printf("1");
                xTimerReset(Blufi_Handle, 0);
//                xSemaphoreGiveFromISR(BLUFI_sem, pdTRUE);
//                xSemaphoreGive(BLUFI_sem);
//                printf("2");
//                set_task_status(EV_BLUFI_TASK);
//                printf("3");
            }
        }
        break;
    case WIFI_EVENT_STA_CONNECTED:
        gl_sta_connected = true;
        event = (wifi_event_sta_connected_t*) event_data;
        memcpy(gl_sta_bssid, event->bssid, 6);
        memcpy(gl_sta_ssid, event->ssid, event->ssid_len);
        gl_sta_ssid_len = event->ssid_len;
        ESP_LOGI(TAG, "wifi connect succuess!\n");

        break;
    case WIFI_EVENT_STA_DISCONNECTED:
    	if (gl_sta_connected == false && example_wifi_reconnect() == false)
    	{
			gl_sta_is_connecting = false;
			disconnected_event = (wifi_event_sta_disconnected_t*) event_data;
			example_record_wifi_conn_info(disconnected_event->rssi, disconnected_event->reason);
		}
        /* This is a workaround as ESP32 WiFi libs don't currently
           auto-reassociate. */
        gl_sta_connected = false;
        memset(gl_sta_ssid, 0, 32);
        memset(gl_sta_bssid, 0, 6);
        gl_sta_ssid_len = 0;
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    case WIFI_EVENT_SCAN_DONE:
    {
        uint16_t apCount = 0;
        esp_wifi_scan_get_ap_num(&apCount);
        if (apCount == 0)
        {
            ESP_LOGI(TAG, "Nothing AP found");
            break;
        }
        wifi_ap_record_t *ap_list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * apCount);
        if (!ap_list)
        {
            ESP_LOGI(TAG, "malloc error, ap_list is NULL");
            break;
        }
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&apCount, ap_list));
        esp_blufi_ap_record_t * blufi_ap_list = (esp_blufi_ap_record_t *)malloc(apCount * sizeof(esp_blufi_ap_record_t));
        if (!blufi_ap_list)
        {
            if (ap_list)
            {
                free(ap_list);
            }
            ESP_LOGI(TAG, "malloc error, blufi_ap_list is NULL");
            break;
        }
        for (int i = 0; i < apCount; ++i)
        {
            blufi_ap_list[i].rssi = ap_list[i].rssi;
            memcpy(blufi_ap_list[i].ssid, ap_list[i].ssid, sizeof(ap_list[i].ssid));
        }

        if (ble_is_connected == true)
        {
            esp_blufi_send_wifi_list(apCount, blufi_ap_list);
        }
        else
        {
            ESP_LOGI(TAG, "BLUFI BLE is not connected yet\n");
        }

        esp_wifi_scan_stop();
        free(ap_list);
        free(blufi_ap_list);
        break;
    }
    default:
        break;
    }
    return;
}

void reset_wifi(void)
{
    printf("reset wifi\n");
    esp_wifi_restore();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg));
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA));

    ESP_ERROR_CHECK( esp_wifi_start());
    WIFI_Handle= xTimerCreate("wifi_timer", 1000 * 20, pdFALSE, NULL, wifi_timer);
}

void Initialise_Wifi(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg));
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA));

    WIFI_Handle = xTimerCreate("wifi_timer", 1000 * 20, pdFALSE, NULL, wifi_timer);
    Blufi_Handle = xTimerCreate("Blufi_timer", 1000 * 60, pdFALSE, NULL, Blufi_timer);

    ESP_ERROR_CHECK(esp_wifi_start());
}
