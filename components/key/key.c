#include "key.h"
#define ESP_INTR_FLAG_DEFAULT 0//定义默认的中断标志为0
#define Key_Gpio_Pin	38
#define Key1_GPIO_PIN   38

static const char *TAG = "key";

static QueueHandle_t gpio_evt_queue = NULL;

extern const uint8_t *bmp_name[7];
extern uint8_t *back;
extern uint8_t num;
extern QueueHandle_t wifi_quent;
//static void EXIT_Handelr(void * args)
//{
//	(void)args;
//	printf("gpio7 interrupt\n");
//}

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void Key_task(void* arg)
{
    uint32_t io_num;
    for(;;)
    {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY))
        {
        	if(io_num == Key_Gpio_Pin && gpio_get_level(Key_Gpio_Pin))
        	{
        		printf("GPIO[%"PRIu32"] intr, val: %d\n", io_num, gpio_get_level(io_num));
        	}
        }
    }
}

void key_config(void)
{
	gpio_config_t io_conf = {};
	//disable interrupt
	io_conf.intr_type = GPIO_INTR_POSEDGE;
	//set as output mode
	io_conf.mode = GPIO_MODE_INPUT;
	//bit mask of the pins that you want to set,e.g.21
	io_conf.pin_bit_mask = (1ULL << Key_Gpio_Pin);
	//disable pull-down mode
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	//disable pull-up mode
	io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
	//configure GPIO with the given settings
	gpio_config(&io_conf);

	//gpio_set_intr_type(7, GPIO_INTR_ANYEDGE);
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

	gpio_isr_handler_add(Key_Gpio_Pin, gpio_isr_handler, (void *)Key_Gpio_Pin);

	gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
	xTaskCreate(Key_task, "Key_task", 2048, NULL, 4, NULL);

	printf("gpio7 input init\n");
}

void key1_task(void *pvParameters)//鎸夐敭浠诲姟锛�
{
    ESP_LOGD(TAG, "enter key_task task");
    int timeout_cnt = 300;
    int send2 = 6;
    while(1)
    {
        if(!gpio_get_level(Key1_GPIO_PIN))
        {
            vTaskDelay(10 / portTICK_PERIOD_MS);

            while(!gpio_get_level(Key1_GPIO_PIN) && timeout_cnt--)
            {
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }

            printf("GPIO 38, val: %d\n", gpio_get_level(Key1_GPIO_PIN));

            num++;
            num %= 7;
            back = bmp_name[num];
            xQueueSend(wifi_quent, &send2, 10000);
//            if(timeout_cnt <= 0)
//            {
//                ESP_LOGD(TAG, "key1 long press, wifi reset\n");
//
//                LED_OFF();
//
//                ESP_LOGD(TAG, "reset wifi\n");
//                esp_wifi_restore();
//                //esp_restart();
//                esp_core_reset();
//            }

            timeout_cnt = 300;
        }
        else
        {
            vTaskDelay(10 / portTICK_PERIOD_MS);//every 10ms scan key;
        }
    }
}

void key1_config(void)
{
    gpio_config_t io_conf = {};
    io_conf.intr_type       = GPIO_INTR_DISABLE;
    io_conf.mode            = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask    = ((uint64_t)1ULL << Key1_GPIO_PIN);
    io_conf.pull_down_en    = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en      = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    xTaskCreate(key1_task, "key1_task", 2048, NULL, 2, NULL);
}
