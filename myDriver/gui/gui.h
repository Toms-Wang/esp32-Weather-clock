#ifndef gui_H
#define gui_H

#include "lcd.h"
#include "tim_data.h"
#include "weather.h"
#include "time.h"

void gui_update_back(const uint8_t *back);
void gui_update_weather(uint16_t xes, uint16_t yes, const uint8_t *back);
void http_update_time(void);
void gui_update_time(uint16_t xes, uint16_t yes, const uint8_t *back);
void gui_update_week(uint16_t xes, uint16_t yes, const uint8_t *back);

#endif
