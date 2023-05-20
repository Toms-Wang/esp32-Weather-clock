#include "LED.h"

void Led_Config(void)
{
	gpio_config_t io_conf = {};
	//disable interrupt
	io_conf.intr_type = GPIO_INTR_DISABLE;
	//set as output mode
	io_conf.mode = GPIO_MODE_OUTPUT;
	//bit mask of the pins that you want to set,e.g.21
	io_conf.pin_bit_mask = (1ULL << BLINK_GPIO);
	//disable pull-down mode
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	//disable pull-up mode
	io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
	//configure GPIO with the given settings
	gpio_config(&io_conf);

	gpio_set_level(BLINK_GPIO, 0);
}

void LED_ON(void)
{
	gpio_set_level(BLINK_GPIO, 1);
}

void LED_OFF(void)
{
	gpio_set_level(BLINK_GPIO, 0);
}

