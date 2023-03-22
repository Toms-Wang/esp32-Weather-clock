#include "gui.h"

#define tem_num 37
uint8_t len = 0;

static const char *TAG1 = "example1";

static char week_num[7][8]= {"周日", "周一", "周二", "周三", "周四", "周五", "周六"};

static char tem_bmp[tem_num][25] = {"晴", "晴夜", "多云", "晴间多云", "晴间多云夜", "大部多云",
						"大部多云夜", "阴", "阵雨", "雷阵雨", "雷阵雨伴有冰雹", "小雨",
						"中雨", "大雨", "暴雨", "大暴雨", "特大暴雨", "冻雨",
						"雨夹雪", "阵雪", "小雪", "中雪", "大雪", "暴雪",
						"浮尘", "扬沙", "沙尘暴", "强沙尘暴", "雾", "霾",
						"风", "大风", "飓风", "热带风暴", "龙卷风", "冷","热"
};

static char tem_name[tem_num][15] = {"/bmp1.txt",  "/bmp2.txt",  "/bmp3.txt",  "/bmp4.txt",  "/bmp5.txt",  "/bmp6.txt",
								 "/bmp7.txt",  "/bmp8.txt",  "/bmp9.txt",  "/bmp10.txt", "/bmp11.txt", "/bmp12.txt",
								 "/bmp13.txt", "/bmp14.txt", "/bmp15.txt", "/bmp16.txt", "/bmp17.txt", "/bmp18.txt",
								 "/bmp19.txt", "/bmp20.txt", "/bmp21.txt", "/bmp22.txt", "/bmp23.txt", "/bmp24.txt",
								 "/bmp25.txt", "/bmp26.txt", "/bmp27.txt", "/bmp28.txt", "/bmp29.txt", "/bmp30.txt",
								 "/bmp31.txt", "/bmp32.txt", "/bmp33.txt", "/bmp34.txt", "/bmp35.txt", "/bmp36.txt", "/bmp37.txt"
};

void gui_update_back(const uint8_t *back)
{
	LCD_Display(0, 0, back);
}

void gui_update_weather(uint8_t xes, uint8_t yes, const uint8_t *back)//更新天气图标；
{
	uint8_t cit[20]  = {0};
	uint8_t wea[20]  = {0};
	uint8_t tem[20]  = {0};
	uint16_t p_x = 0;
	uint16_t p_y = 0;

	if(https_get_weather(cit, wea, tem))
	{
		ESP_LOGI(TAG1, "weather update success");
		ESP_LOGI(TAG1, "cit = %s", cit);
		ESP_LOGI(TAG1, "wea = %s", wea);
		ESP_LOGI(TAG1, "tem = %s", tem);

		for(uint8_t i = 0; i < tem_num; i++)//比对天气中文值；
		{
			if(strcmp((char *)tem_bmp[i], (char *)wea) == 0)
			{
				if(i == 0 || i == 3 || i == 5)//有夜晚图标的，切换夜晚图标；
				{
					struct tm* tm3 = get_tm_time();
					if(tm3->tm_hour < 6 || tm3->tm_hour > 18)
					{
						i++;
					}
				}
				ESP_LOGI(TAG1, "weather = %s\n", (char *)tem_bmp[i]);
				LCD_Display_bmp(xes, yes, (char *)tem_name[i], back, &p_x, &p_y);//更新天气图案；
				break;
			}
		}

		if(len > ((strlen((char *)wea) * 2 / 3) + strlen((char *)tem)))
		{
			Display_CE_bc(p_x - 60, p_y, (char *)"               ", WHITE, back);
			Display_CE_bc(p_x - 60, p_y + 16 + 8, (char *)"               ", WHITE, back);
		}

		len = ((strlen((char *)wea) * 2 / 3) + strlen((char *)tem));

		if((strlen((char *)wea) /3 ) > 5)//考虑天气情况长度过长，对其换行；
		{
			Display_CE_bc(p_x - strlen((char*)wea) / 3 * 16 / 2, p_y, (char *)wea, WHITE, back);

			Display_CE_bc(p_x - (strlen((char *)tem) + 2) * 8 / 2, p_y + 8 + 16, (char *)tem, WHITE, back);
			//中文“℃”显示；
			LCD_ShowChinese_C_bc(p_x - (strlen((char *)tem) + 2) * 8 / 2 + strlen((char *)tem) * 8, p_y + 8 + 16 , 0xA1, 0xE6, WHITE, back);
		}
		else
		{
			Display_CE_bc(p_x - (strlen((char*)wea) / 3 * 16 + (3 + strlen((char *)tem)) * 8) / 2, p_y, (char *)wea, WHITE, back);

			Display_CE_bc(p_x + (strlen((char*)wea) / 3 * 16 + (3 + strlen((char *)tem)) * 8) / 2 - (2 + strlen((char *)tem)) * 8 , p_y, (char *)tem, WHITE, back);
			LCD_ShowChinese_C_bc(p_x + (strlen((char*)wea) / 3 * 16 + (3 + strlen((char *)tem)) * 8) / 2 - 2 * 8, p_y, 0xA1, 0xE6, WHITE, back);
		}

		memset((char *)tem, 0, strlen((char *)tem));
	}
	else
	{
		ESP_LOGI(TAG1, "weather update fail");
	}
}

void http_update_time(void)//联网更新时间；
{
	char tim[50] = {0};
	if(http_get_time2(tim))//获取时间；
	{
		ESP_LOGI(TAG1, "time update success");
		setTime_str(tim);//更新时间；
	}
	else
	{
		ESP_LOGI(TAG1, "time update fail");
	}
}

void gui_update_time(uint8_t xes, uint8_t yes, const uint8_t *back)//显示时间
{
	struct tm* tm3;
	time_str_t time0;
	uint8_t mon = 0;

	tm3 = get_tm_time();

	if(tm3->tm_hour == 0 && tm3->tm_min == 0)//00:00联网更新一次（也可能是三次）时间；
	{
		http_update_time();
		tm3 = get_tm_time();
	}

	ESP_LOGI(TAG1, "display time = %s", asctime(tm3));

	sprintf(time0.hour, "%02d", tm3->tm_hour);
	sprintf(time0.min, "%02d", tm3->tm_min);
	mon = tm3->tm_mon + 1;
	sprintf(time0.mon, "%d", mon);
	sprintf(time0.day, "%02d", tm3->tm_mday);

	LCD_showStr48_bc(xes + 30, yes + 16, time0.hour, WHITE, back);
	LCD_showStr48_bc(xes + 30, yes + 64, time0.min, WHITE, back);

	//Display_CE_bc(8 , 120, (char *)"03月20日 周一", WHITE, gImage_bmp320);
	if(strlen(time0.mon) == 1)
	{
		Display_CE_bc(xes + 8, yes + 120, " ", WHITE, back);
		Display_CE_bc(xes + 16, yes + 120, time0.mon, WHITE, back);
	}
	else if(strlen(time0.mon) == 2)
	{
		Display_CE_bc(xes + 8, yes + 120, time0.mon, WHITE, back);
	}

	//Display_CE_bc(xes + 8 + (2 - strlen(time0.mon)) * 8 , yes + 120, time0.mon, WHITE, back);
	Display_CE_bc(xes + 24, yes + 120, "月", WHITE, back);
	Display_CE_bc(xes + 40, yes + 120, time0.day, WHITE, back);
	Display_CE_bc(xes + 56, yes + 120, "日", WHITE, back);

	if(tm3->tm_wday >= 0 && tm3->tm_wday < 7)
	{
		Display_CE_bc(xes + 80 , yes + 120, week_num[tm3->tm_wday], WHITE, back);
	}
}
