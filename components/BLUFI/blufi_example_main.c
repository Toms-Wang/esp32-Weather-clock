/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


/****************************************************************************
* This is a demo for bluetooth config wifi connection to ap. You can config ESP32 to connect a softap
* or config ESP32 as a softap to be connected by other device. APP can be downloaded from github
* android source code: https://github.com/EspressifApp/EspBlufi
* iOS source code: https://github.com/EspressifApp/EspBlufiForiOS
****************************************************************************/
#include "blufi_example_main.h"

#define WIFI_LIST_NUM   10

extern SemaphoreHandle_t BLUFI_sem;

static const char* TAG = "Blufi";
extern uint8_t  blufi_state;

/* store the station info for send back to phone */
extern bool gl_sta_connected;
static bool gl_sta_got_ip = false;
bool ble_is_connected = false;
extern uint8_t gl_sta_bssid[6];
extern uint8_t gl_sta_ssid[32];
extern int gl_sta_ssid_len;
extern wifi_config_t sta_config;
extern bool gl_sta_is_connecting;
extern esp_blufi_extra_info_t gl_sta_conn_info;

extern TimerHandle_t WIFI_Handle;

uint8_t blufi_is_first = 0;
//uint32_t Default_RTC_Sleep_Sec = 5 * 60;
uint8_t  Custom_state = 0;
char Custom_data[40] = {0};


void ble_close(void);
static void example_event_callback(esp_blufi_cb_event_t event, esp_blufi_cb_param_t *param);


static uint8_t  blufi_service_uuid128[32] =
{
    /* LSB <--------------------------------------------------------------------------------> MSB */
    //first uuid, 16bit, [12],[13] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00,
};

static esp_ble_adv_data_t blufi_adv_data =
{
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x0006, //slave connection min interval, Time = min_interval * 1.25 msec
    .max_interval = 0x0010, //slave connection max interval, Time = max_interval * 1.25 msec
    .appearance = 0x00,
    .manufacturer_len = 0,
    .p_manufacturer_data =  NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = 16,
    .p_service_uuid = blufi_service_uuid128,
    .flag = 0x6,
};

void BLUFI_INIT_Task(void * arg)
{
    for(;;)
    {
        if(xSemaphoreTake(BLUFI_sem, portMAX_DELAY) == pdTRUE)
        {
        	printf("Enter BLUFI_Task\n");
            blufi_config();
        }
    }
}

void blufi_adv_start(void)
{
    char Blufi_device_name[20] = {0};
    strcpy(Blufi_device_name, "Disp_");
    read_mac_address(Blufi_device_name + 5);
    esp_ble_gap_set_device_name(Blufi_device_name);
    esp_ble_gap_config_adv_data(&blufi_adv_data);
}

void wifi_ble_close(void)
{
    esp_wifi_disconnect();
    ESP_ERROR_CHECK(esp_wifi_stop());

    if(blufi_state == 1)
    {
        ESP_ERROR_CHECK(esp_bt_controller_disable());
    }
}

void ble_close(void)
{
	if(blufi_state == 1)
	{
		ESP_ERROR_CHECK(esp_bt_controller_disable());
	}
}

static esp_blufi_callbacks_t example_callbacks =
{
    .event_cb = example_event_callback,
    .negotiate_data_handler = blufi_dh_negotiate_data_handler,
    .encrypt_func = blufi_aes_encrypt,
    .decrypt_func = blufi_aes_decrypt,
    .checksum_func = blufi_crc_checksum,
};

static void example_event_callback(esp_blufi_cb_event_t event, esp_blufi_cb_param_t *param)
{
    /* actually, should post to blufi_task handle the procedure,
     * now, as a example, we do it more simply */
    switch (event)
    {
    case ESP_BLUFI_EVENT_INIT_FINISH:
        BLUFI_INFO("BLUFI init finish\n");
        //esp_blufi_adv_start();
        blufi_adv_start();
        break;
    case ESP_BLUFI_EVENT_DEINIT_FINISH:
        BLUFI_INFO("BLUFI deinit finish\n");
        break;
    case ESP_BLUFI_EVENT_BLE_CONNECT:
        BLUFI_INFO("BLUFI ble connect\n");
        ble_is_connected = true;
        esp_blufi_adv_stop();
        blufi_security_init();

        break;
    case ESP_BLUFI_EVENT_BLE_DISCONNECT:
        BLUFI_INFO("BLUFI ble disconnect\n");
        ble_is_connected = false;
        blufi_security_deinit();
//        esp_blufi_adv_start();
        blufi_adv_start();
        break;
    case ESP_BLUFI_EVENT_SET_WIFI_OPMODE:
        BLUFI_INFO("BLUFI Set WIFI opmode %d\n", param->wifi_mode.op_mode);
        ESP_ERROR_CHECK( esp_wifi_set_mode(param->wifi_mode.op_mode));
        break;
    case ESP_BLUFI_EVENT_REQ_CONNECT_TO_AP:
        BLUFI_INFO("BLUFI requset wifi connect to AP\n");
        /* there is no wifi callback when the device has already connected to this wifi
        so disconnect wifi before connection.
        */
        esp_wifi_disconnect();

        xTimerReset(WIFI_Handle, 0);
        esp_err_t errt = esp_wifi_connect();
        printf("errt = 0x%x\n", errt);
        break;
    case ESP_BLUFI_EVENT_REQ_DISCONNECT_FROM_AP:
        BLUFI_INFO("BLUFI requset wifi disconnect from AP\n");
        esp_wifi_disconnect();
        break;
    case ESP_BLUFI_EVENT_REPORT_ERROR:
        BLUFI_ERROR("BLUFI report error, error code %d\n", param->report_error.state);
        esp_blufi_send_error_info(param->report_error.state);
        break;
    case ESP_BLUFI_EVENT_GET_WIFI_STATUS: {
        wifi_mode_t mode;
        esp_blufi_extra_info_t info;

        esp_wifi_get_mode(&mode);

        if (gl_sta_connected)
        {
            memset(&info, 0, sizeof(esp_blufi_extra_info_t));
            memcpy(info.sta_bssid, gl_sta_bssid, 6);
            info.sta_bssid_set = true;
            info.sta_ssid = gl_sta_ssid;
            info.sta_ssid_len = gl_sta_ssid_len;
            esp_blufi_send_wifi_conn_report(mode, gl_sta_got_ip ? ESP_BLUFI_STA_CONN_SUCCESS : ESP_BLUFI_STA_NO_IP, softap_get_current_connection_number(), &info);
        }
        else if (gl_sta_is_connecting)
        {
        	esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONNECTING, softap_get_current_connection_number(), &gl_sta_conn_info);
		}
        else
        {
        	esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_FAIL, softap_get_current_connection_number(), &gl_sta_conn_info);
		}
        BLUFI_INFO("BLUFI get wifi status from AP\n");

        break;
    }
    case ESP_BLUFI_EVENT_RECV_SLAVE_DISCONNECT_BLE:
        BLUFI_INFO("blufi close a gatt connection");
        esp_blufi_disconnect();
        break;
    case ESP_BLUFI_EVENT_DEAUTHENTICATE_STA:
        /* TODO */
        break;
    case ESP_BLUFI_EVENT_RECV_STA_BSSID:
        memcpy(sta_config.sta.bssid, param->sta_bssid.bssid, 6);
        sta_config.sta.bssid_set = 1;
        esp_wifi_set_config(WIFI_IF_STA, &sta_config);
        BLUFI_INFO("Recv STA BSSID %s\n", sta_config.sta.ssid);
        break;
    case ESP_BLUFI_EVENT_RECV_STA_SSID:
        strncpy((char *)sta_config.sta.ssid, (char *)param->sta_ssid.ssid, param->sta_ssid.ssid_len);
        sta_config.sta.ssid[param->sta_ssid.ssid_len] = '\0';
        esp_wifi_set_config(WIFI_IF_STA, &sta_config);
        BLUFI_INFO("Recv STA SSID %s\n", sta_config.sta.ssid);
        break;
    case ESP_BLUFI_EVENT_RECV_STA_PASSWD:
        strncpy((char *)sta_config.sta.password, (char *)param->sta_passwd.passwd, param->sta_passwd.passwd_len);
        sta_config.sta.password[param->sta_passwd.passwd_len] = '\0';
        esp_wifi_set_config(WIFI_IF_STA, &sta_config);
        BLUFI_INFO("Recv STA PASSWORD %s\n", sta_config.sta.password);
        break;

    case ESP_BLUFI_EVENT_GET_WIFI_LIST:
    {
        wifi_scan_config_t scanConf =
        {
            .ssid = NULL,
            .bssid = NULL,
            .channel = 0,
            .show_hidden = false
        };
        esp_wifi_scan_start(&scanConf, true);

        BLUFI_INFO("BLUFI GET WIFI LIST \n");//get wifi list;
        break;
    }
    case ESP_BLUFI_EVENT_RECV_CUSTOM_DATA:
    	//ESP_LOGI(TAG, "Recv Custom Data %d\n", param->custom_data.data_len);
        esp_log_buffer_hex("Custom Data", param->custom_data.data, param->custom_data.data_len);
//        esp_blufi_send_custom_data(param->custom_data.data, param->custom_data.data_len);//blufi send;
        if(param->custom_data.data_len >= 2 && param->custom_data.data_len <= 3)
        {
            if(param->custom_data.data[0] == 's' || param->custom_data.data[0] == 'S')
            {
                for(int i = 1; i < param->custom_data.data_len; i++)
                {
                    if(param->custom_data.data[i] > '9' || param->custom_data.data[i] < '0')
                    {
                        Custom_state = 1;
                        break;
                    }
                }

                if(!Custom_state)
                {
                    int sec = atoi((char *)&param->custom_data.data[1]);

//                  Default_RTC_Sleep_Sec = sec;
//                    NVS_write_RTCtime(sec);
//                    set_sleeptime(sec);

                    memset(Custom_data, 0 ,sizeof(Custom_data));
                    sprintf(Custom_data, "Sleep time:%d seconds", sec);
                    esp_blufi_send_custom_data((uint8_t *)Custom_data, strlen(Custom_data));
                }
                else
                {
                    Custom_state = 0;
                }
            }
            else if(param->custom_data.data[0] == 'm'  ||  param->custom_data.data[0] == 'M')
            {
                for(int i = 1; i < param->custom_data.data_len; i++)
                {
                    if(param->custom_data.data[i] > '9' || param->custom_data.data[i] < '0')
                    {
                        Custom_state = 1;
                        break;
                    }
                }

                if(!Custom_state)
                {
                    int min = atoi((char *)&param->custom_data.data[1]);
//                  Default_RTC_Sleep_Sec = min * 60;
//                  NVS_write_RTCtime(Default_RTC_Sleep_Sec);

//                    NVS_write_RTCtime(min * 60);
//                    set_sleeptime(min * 60);

                    memset(Custom_data, 0 ,sizeof(Custom_data));
                    sprintf(Custom_data, "Sleep time:%d minutes", min);
                    esp_blufi_send_custom_data((uint8_t *)Custom_data, strlen(Custom_data));
                }
                else
                {
                    Custom_state = 0;
                }

            }
            else if(param->custom_data.data[0] == 'h'  ||  param->custom_data.data[0] == 'H')
            {
                for(int i = 1; i < param->custom_data.data_len; i++)
                {
                    if(param->custom_data.data[i] > '9' || param->custom_data.data[i] < '0')
                    {
                        Custom_state = 1;
                        break;
                    }
                }

                if(!Custom_state)
                {
                    int hour = atoi((char *)&param->custom_data.data[1]);
//                  Default_RTC_Sleep_Sec = hour * 60 * 60;
//                  NVS_write_RTCtime(Default_RTC_Sleep_Sec);

//                    NVS_write_RTCtime(hour * 60 * 60);
//                    set_sleeptime(hour * 60 * 60);

                    memset(Custom_data, 0 ,sizeof(Custom_data));
                    sprintf(Custom_data, "Sleep time:%d hours", hour);
                    esp_blufi_send_custom_data((uint8_t * )Custom_data, strlen(Custom_data));
                }
                else
                {
                    Custom_state = 0;
                }
            }
        }
        break;
    case ESP_BLUFI_EVENT_RECV_USERNAME:
        /* Not handle currently */
        break;
    case ESP_BLUFI_EVENT_RECV_CA_CERT:
        /* Not handle currently */
        break;
    case ESP_BLUFI_EVENT_RECV_CLIENT_CERT:
        /* Not handle currently */
        break;
    case ESP_BLUFI_EVENT_RECV_SERVER_CERT:
        /* Not handle currently */
        break;
    case ESP_BLUFI_EVENT_RECV_CLIENT_PRIV_KEY:
        /* Not handle currently */
        break;;
    case ESP_BLUFI_EVENT_RECV_SERVER_PRIV_KEY:
        /* Not handle currently */
        break;
    default:
        break;
    }
}

void blufi_config(void)
{
	printf("blufi config\n");
    esp_err_t ret;

    if(!blufi_is_first)
    {
    	ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    	blufi_is_first = 1;
    }

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        BLUFI_ERROR("%s initialize bt controller failed: %s\n", __func__, esp_err_to_name(ret));
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        BLUFI_ERROR("%s enable bt controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_blufi_host_and_cb_init(&example_callbacks);
    if (ret) {
        BLUFI_ERROR("%s initialise failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    BLUFI_INFO("BLUFI VERSION %04x\n", esp_blufi_get_version());
}

esp_err_t ble_deinit(void)
{
    int ret;

//    printf("1\n");
    ret = esp_blufi_profile_deinit();
    if (ret)
    {
        ESP_LOGD(TAG, "%s deinit  blufi profile failed: %s\n", __func__, esp_err_to_name(ret));
        return -1;
    }

//    printf("2\n");
    blufi_security_deinit();
//    printf("3\n");
    ret = esp_bluedroid_disable();
    if (ret)
    {
        ESP_LOGD(TAG, "%s disable bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return -1;
    }
//    printf("4\n");
    ret = esp_bluedroid_deinit();
    if (ret) {
        ESP_LOGD(TAG, "%s deinit bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return -1;
    }
//    printf("5\n");
    ret = esp_bt_controller_disable();
    if (ret) {
        ESP_LOGD(TAG, "%s disable controller failed: %s\n", __func__, esp_err_to_name(ret));
        return -1;
    }
//    printf("6\n");
    ret = esp_bt_controller_deinit();
    if (ret) {
        ESP_LOGD(TAG, "%s deinit controller failed: %s\n", __func__, esp_err_to_name(ret));
        return -1;
    }
//    printf("7\n");
    ESP_LOGD(TAG, "ble deinit completed\n");
    return 0;
}
