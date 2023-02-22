#ifndef LED_H_
#define LED_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_system.h"
#include "driver/gpio.h"

#define BLINK_GPIO		2

void Led_Config(void);
void LED_ON(void);
void LED_OFF(void);

#endif
