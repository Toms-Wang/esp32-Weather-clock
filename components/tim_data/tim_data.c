#include "tim_data.h"

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
#include "esp_http_client.h"


static const char *TAG = "example";

#define MAX_HTTP_OUTPUT_BUFFER 2048
#define TIME_URL "http://quan.suning.com/getSysTime.do"
#define TIME_URL2 "http://api.k780.com/?app=life.time&appkey=10003&sign=b59bc3ef6191eb9f747dd4e83c99f2a4&format=json"

char output_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};   //用于接收通过http协议返回的数据

uint8_t http_get_time(char * pxtim)
{
	uint8_t get_time_status = 0;
//02-1 定义需要的变量
    char output_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};   //用于接收通过http协议返回的数据
    int content_length = 0;  //http协议头的长度


    //02-2 配置http结构体

   //定义http配置结构体，并且进行清零
    esp_http_client_config_t config ;
    memset(&config,0,sizeof(config));

    //向配置结构体内部写入url


    //static const char *URL = TIME_URL;
    config.url = TIME_URL;

    //初始化结构体
    esp_http_client_handle_t client = esp_http_client_init(&config);	//初始化http连接

    //设置发送请求
    esp_http_client_set_method(client, HTTP_METHOD_GET);

    //02-3 循环通讯

    // 与目标主机创建连接，并且声明写入内容长度为0
    esp_err_t err = esp_http_client_open(client, 0);

    //如果连接失败
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    }
    //如果连接成功
    else
    {

        //读取目标主机的返回内容的协议头
        content_length = esp_http_client_fetch_headers(client);

        //如果协议头长度小于0，说明没有成功读取到
        if (content_length < 0)
        {
            ESP_LOGE(TAG, "HTTP client fetch headers failed");
        }

        //如果成功读取到了协议头
        else
        {

            //读取目标主机通过http的响应内容
            int data_read = esp_http_client_read_response(client, output_buffer, MAX_HTTP_OUTPUT_BUFFER);
            if (data_read >= 0)
            {
            	get_time_status = 1;
                //打印响应内容，包括响应状态，响应体长度及其内容
                ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %lld",
                esp_http_client_get_status_code(client),				//获取响应状态信息
                esp_http_client_get_content_length(client));			//获取响应信息长度
                printf("data:%s\n", output_buffer);
				//对接收到的数据作相应的处理
                cJSON* root = NULL;
                root = cJSON_Parse(output_buffer);

                cJSON* time =cJSON_GetObjectItem(root,"sysTime2");

                printf("%s\n",time->valuestring);

                memset(pxtim, 0, strlen(pxtim));
                strcpy(pxtim, time->valuestring);

                cJSON_Delete(root);

            }
            //如果不成功
            else
            {
                ESP_LOGE(TAG, "Failed to read response");
            }
        }
    }

    //关闭连接
    esp_http_client_close(client);
    return get_time_status;

}

uint8_t http_get_time2(char * pxtim)
{
//	printf("1\n");
	uint8_t get_time_status = 0;

//    char output_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};   //用于接收通过http协议返回的数据
    int content_length = 0;  //http协议头的长度

   //定义http配置结构体，并且进行清零
    esp_http_client_config_t config ;
    memset(&config,0,sizeof(config));

    //向配置结构体内部写入url
//    printf("2\n");

    //static const char *URL = TIME_URL;
    config.url = TIME_URL2;

    //初始化结构体
    esp_http_client_handle_t client = esp_http_client_init(&config);	//初始化http连接
//    printf("3\n");
    //设置发送请求
    esp_http_client_set_method(client, HTTP_METHOD_GET);
//    printf("4\n");
    //02-3 循环通讯

    // 与目标主机创建连接，并且声明写入内容长度为0
    esp_err_t err = esp_http_client_open(client, 0);

//    printf("5\n");
    //如果连接失败
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    }
    //如果连接成功
    else
    {
//    	printf("6\n");
        //读取目标主机的返回内容的协议头
        content_length = esp_http_client_fetch_headers(client);
//        printf("7\n");
        //如果协议头长度小于0，说明没有成功读取到
        if (content_length < 0)
        {
            ESP_LOGE(TAG, "HTTP client fetch headers failed");
        }

        //如果成功读取到了协议头
        else
        {
            //读取目标主机通过http的响应内容
            int data_read = esp_http_client_read_response(client, output_buffer, MAX_HTTP_OUTPUT_BUFFER);
            if (data_read >= 0)
            {
            	get_time_status = 1;
                //打印响应内容，包括响应状态，响应体长度及其内容
                ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %lld",
                esp_http_client_get_status_code(client),				//获取响应状态信息
                esp_http_client_get_content_length(client));			//获取响应信息长度
                printf("data:%s\n", output_buffer);
				//对接收到的数据作相应的处理
                cJSON* root = NULL;
                root = cJSON_Parse(output_buffer);
                if(root != NULL)
                {
                	cJSON* cjson_result =cJSON_GetObjectItem(root, "result");
                	if(cjson_result != NULL)
                	{
                		cJSON* cjson_time =cJSON_GetObjectItem(cjson_result, "datetime_1");
                		printf("%s\n",cjson_time->valuestring);

						memset(pxtim, 0, strlen(pxtim));
						strcpy(pxtim, cjson_time->valuestring);

						cJSON* cjson_week =cJSON_GetObjectItem(cjson_result, "week_1");
						printf("week = %s\n", cjson_week->valuestring);

						strcpy(pxtim + strlen(cjson_time->valuestring), cjson_week->valuestring);

						printf("time = %s\n", pxtim);
                	}
                }
                cJSON_Delete(root);
            }
            //如果不成功
            else
            {
                ESP_LOGE(TAG, "Failed to read response");
            }
        }
    }

    //关闭连接
    esp_http_client_close(client);
    return get_time_status;

}

void setTime_str(char * pxtim)
{
	time_str_t tim_str1;
	struct tm t = {0};

	memcpy(tim_str1.year, pxtim, 4);
	t.tm_year = atoi(tim_str1.year) - 1900;
	memcpy(tim_str1.mon, pxtim + 5, 2);
	t.tm_mon = atoi(tim_str1.mon) - 1;
	memcpy(tim_str1.day, pxtim + 8, 2);
	t.tm_mday = atoi(tim_str1.day);
	memcpy(tim_str1.hour, pxtim + 11, 2);
	t.tm_hour = atoi(tim_str1.hour);
	memcpy(tim_str1.min, pxtim + 14, 2);
	t.tm_min = atoi(tim_str1.min);
	memcpy(tim_str1.sec, pxtim + 17, 2);
	t.tm_sec = atoi(tim_str1.sec);
	memcpy(tim_str1.week, pxtim + 19, 1);
	t.tm_wday =atoi(tim_str1.week);

    time_t timeSinceEpoch = mktime(&t);
    //setTime(timeSinceEpoch, ms);
    struct timeval now = { .tv_sec = timeSinceEpoch };
    settimeofday(&now, NULL);
}

struct tm * get_tm_time(void)
{
	time_t timer;
	struct tm *tblock;
	timer = time(NULL);
	tblock = localtime(&timer);
//	if(tblock != NULL)
//	{
//		return 1;
//	}
	//printf("Local time is: %s\n", asctime(pxtm));
	return tblock;
}
