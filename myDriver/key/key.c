#include "key.h"
#define ESP_INTR_FLAG_DEFAULT 0//定义默认的中断标志为0

static void EXIT_Handelr(void * args)
{
	(void)args;
	printf("gpio7 interrupt\n");
}

void key_config(void)
{
	gpio_config_t io_conf = {};
	//disable interrupt
	io_conf.intr_type = GPIO_INTR_POSEDGE;
	//set as output mode
	io_conf.mode = GPIO_MODE_INPUT;
	//bit mask of the pins that you want to set,e.g.21
	io_conf.pin_bit_mask = (1ULL << 7);
	//disable pull-down mode
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	//disable pull-up mode
	io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
	//configure GPIO with the given settings
	gpio_config(&io_conf);

	//gpio_set_intr_type(7, GPIO_INTR_ANYEDGE);
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

	gpio_isr_handler_add(7, EXIT_Handelr, NULL);

	printf("gpio7 input init\n");
}
