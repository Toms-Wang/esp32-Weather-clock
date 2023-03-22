#ifndef gui_H
#define gui_H

#include "lcd.h"
#include "tim_data.h"
#include "weather.h"
#include "time.h"

void gui_update_back(const uint8_t *back);
void gui_update_weather(uint8_t xes, uint8_t yes, const uint8_t *back);
void http_update_time(void);
void gui_update_time(uint8_t xes, uint8_t yes, const uint8_t *back);

#endif
