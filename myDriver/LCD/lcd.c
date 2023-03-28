#include "lcd.h"
#include "font.h"
#include "hzk.h"
#include "ff.h"
#include "gbk.h"

#define MOUNT_POINT "/sdcard"
#define SPI_MAX_NUM (1024 * 10)

uint8_t PARALLEL_LINES = 16;
spi_device_handle_t spi;
uint16_t BACK_COLOR = WHITE;//默认背景色；
uint8_t color_t[LCD_W * 2] = {0};

uint8_t bmp52[28810];//天气图标缓冲区；
uint8_t bmp53[28810];

void lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc=(int)t->user;
    gpio_set_level(PIN_NUM_DC, dc);
}

void spi_LCD_init(void)
{
	esp_err_t ret;

	spi_bus_config_t buscfg =
	{
		.miso_io_num=PIN_NUM_MISO,
		.mosi_io_num=PIN_NUM_MOSI,
		.sclk_io_num=PIN_NUM_CLK,
		.quadwp_io_num=-1,
		.quadhd_io_num=-1,
		.max_transfer_sz = SPI_MAX_NUM + 8
	};

	spi_device_interface_config_t devcfg =
	{
		.clock_speed_hz=40*1000*1000,
		.mode=2,                                //SPI mode 0
		.spics_io_num=PIN_NUM_CS,               //CS pin
		.queue_size=7,                          //We want to be able to queue 7 transactions at a time
		.pre_cb=lcd_spi_pre_transfer_callback,  //Specify pre-transfer callback to handle D/C line
	};

	//Initialize the SPI bus
	ret=spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);
	ESP_ERROR_CHECK(ret);
	//Attach the LCD to the SPI bus
	ret=spi_bus_add_device(LCD_HOST, &devcfg, &spi);
	ESP_ERROR_CHECK(ret);
}

void LCD_GPIO_Init(void)
{
	gpio_config_t io_conf = {};
	//disable interrupt
	io_conf.intr_type = GPIO_INTR_DISABLE;
	//set as output mode
	io_conf.mode = GPIO_MODE_OUTPUT;
	//bit mask of the pins that you want to set,e.g.21
	io_conf.pin_bit_mask = (1ULL << PIN_NUM_DC) | (1ULL << PIN_NUM_RST) | (1ULL << PIN_NUM_BCKL);
	//disable pull-down mode
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	//disable pull-up mode
	io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
	//configure GPIO with the given settings
	gpio_config(&io_conf);

	gpio_set_level(PIN_NUM_RST, 0);
	gpio_set_level(PIN_NUM_DC, 0);
	gpio_set_level(PIN_NUM_BCKL, 0);
}

void lcd_cmd(const uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&cmd;               //The data is the cmd itself
    t.user=(void*)0;                //D/C needs to be set to 0
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

void lcd_data(const uint8_t data)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer=&data;               //Data
    t.user=(void*)1;                //D/C needs to be set to 1
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

void lcd_color(const uint16_t color)
{
	uint8_t col[2];
	col[0] = color >> 8;
	col[1] = color & 0xFF;
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=2 * 8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer=col;               //Data
    t.user=(void*)1;                //D/C needs to be set to 1
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

void lcd_long_data(const uint8_t *pdata, int len)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=len * 8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer = pdata;               //Data
    t.user=(void*)1;                //D/C needs to be set to 1
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

void LCD_setAddress(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	lcd_cmd(0x2a);
	lcd_color(x1);
	lcd_color(x2);
	lcd_cmd(0x2b);
	lcd_color(y1);
	lcd_color(y2);
	lcd_cmd(0x2c);
}

void LCD_clear(uint16_t color)
{
	uint16_t i, j;

	BACK_COLOR = color;

	LCD_setAddress(0, 0, LCD_W-1, LCD_H-1);

	for(i=0; i<LCD_W; i++)
	{
		for(j=0; j<LCD_H; j++)
		{
			lcd_color(color);
		}
	}
}

void LCD_Fill(uint16_t color)
{
	uint8_t t1 = color >> 8;
	uint8_t t2 = color & 0xFF;
	BACK_COLOR = color;

	for(uint16_t i = 0; i < LCD_W; i++)
	{
		color_t[i * 2]		= t1;
		color_t[i * 2 + 1] 	= t2;
	}

	LCD_setAddress(0, 0, LCD_W-1, LCD_H-1);

	for(uint16_t j=0; j<LCD_H; j++)
	{
		//gpio_set_level(PIN_NUM_DC, 1);
		lcd_long_data(color_t, LCD_W * 2);
	}
}

void LCD_Config_ST7789(void)
{
	spi_LCD_init();//初始化SPI;
	LCD_GPIO_Init();//初始化GPIO;
	//LCD_RES_Clr();
	gpio_set_level(PIN_NUM_RST, 0);
	vTaskDelay(100 / portTICK_RATE_MS);
	//LCD_RES_Set();
	gpio_set_level(PIN_NUM_RST, 1);
	vTaskDelay(100 / portTICK_RATE_MS);



	lcd_cmd(0x11);
	vTaskDelay(120 / portTICK_RATE_MS);

	lcd_cmd(0xB2);
	lcd_data(0x0C);
	lcd_data(0x0C);
	lcd_data(0x00);
	lcd_data(0x33);
	lcd_data(0x33);

	lcd_cmd(0x35);
	lcd_data(0x00);

	lcd_cmd(0x36);
	lcd_data(0x00);

	lcd_cmd(0x3A);
	lcd_data(0x55);

	lcd_cmd(0xB7);
	lcd_data(0x35);

	lcd_cmd(0xBB);
	lcd_data(0x2D);

	lcd_cmd(0xC0);
	lcd_data(0x2C);

	lcd_cmd(0xC2);
	lcd_data(0x01);

	lcd_cmd(0xC3);
	lcd_data(0x15);

	lcd_cmd(0xC4);
	lcd_data(0x20);

	lcd_cmd(0xC6);
	lcd_data(0x01);
	//lcd_data(0x0F);

	lcd_cmd(0xD0);
	lcd_data(0xA4);
	lcd_data(0xA1);

	lcd_cmd(0xD6);
	lcd_data(0xA1);

	lcd_cmd(0xE0);
	lcd_data(0x70);
	lcd_data(0x05);
	lcd_data(0x0A);
	lcd_data(0x0B);
	lcd_data(0x0A);
	lcd_data(0x27);
	lcd_data(0x2F);
	lcd_data(0x44);
	lcd_data(0x47);
	lcd_data(0x37);
	lcd_data(0x14);
	lcd_data(0x14);
	lcd_data(0x29);
	lcd_data(0x2F);

	lcd_cmd(0xE1);
	lcd_data(0x70);
	lcd_data(0x07);
	lcd_data(0x0C);
	lcd_data(0x08);
	lcd_data(0x08);
	lcd_data(0x04);
	lcd_data(0x2F);
	lcd_data(0x33);
	lcd_data(0x46);
	lcd_data(0x18);
	lcd_data(0x15);
	lcd_data(0x15);
	lcd_data(0x2B);
	lcd_data(0x2D);

	vTaskDelay(100 / portTICK_RATE_MS);
	gpio_set_level(PIN_NUM_BCKL, 1);

	lcd_cmd(0x21);

	lcd_cmd(0x29);

	lcd_cmd(0x2C);
}

void LCD_Config(void)
{
	spi_LCD_init();//初始化SPI;
	LCD_GPIO_Init();//初始化GPIO;
	//LCD_RES_Clr();
	gpio_set_level(PIN_NUM_RST, 0);
	vTaskDelay(100 / portTICK_RATE_MS);
	//LCD_RES_Set();
	gpio_set_level(PIN_NUM_RST, 1);
	vTaskDelay(100 / portTICK_RATE_MS);

	gpio_set_level(PIN_NUM_BCKL, 1);
	vTaskDelay(100 / portTICK_RATE_MS);

	lcd_cmd(0x11);
	vTaskDelay(120 / portTICK_RATE_MS);

	//ST7735R Frame Rate
	lcd_cmd(0xB1);
	lcd_data(0x05);
	lcd_data(0x3C);
	lcd_data(0x3C);
	lcd_cmd(0xB2);
	lcd_data(0x05);
	lcd_data(0x3C);
	lcd_data(0x3C);
	lcd_cmd(0xB3);
	lcd_data(0x05);
	lcd_data(0x3C);
	lcd_data(0x3C);
	lcd_data(0x05);
	lcd_data(0x3C);
	lcd_data(0x3C);
	//------------------------------------End ST7735S Frame Rate---------------------------------//
	lcd_cmd(0xB4); //Dot inversion
	lcd_data(0x03);
	//------------------------------------ST7735S Power Sequence---------------------------------//
	lcd_cmd(0xC0);
	lcd_data(0x28);
	lcd_data(0x08);
	lcd_data(0x04);
	lcd_cmd(0xC1);
	lcd_data(0XC0);
	lcd_cmd(0xC2);
	lcd_data(0x0D);
	lcd_data(0x00);
	lcd_cmd(0xC3);
	lcd_data(0x8D);
	lcd_data(0x2A);
	lcd_cmd(0xC4);
	lcd_data(0x8D);
	lcd_data(0xEE);
	//---------------------------------End ST7735S Power Sequence-------------------------------------//
	lcd_cmd(0xC5); //VCOM
	lcd_data(0x0A);

	lcd_cmd(0x36); //MX, MY, RGB mode
	lcd_data(0xC8);

	//lcd_data(0xA8);
	//------------------------------------ST7735S Gamma Sequence---------------------------------//
	lcd_cmd(0xE0);
	lcd_data(0x04);
	lcd_data(0x22);
	lcd_data(0x07);
	lcd_data(0x0A);
	lcd_data(0x2E);
	lcd_data(0x30);
	lcd_data(0x25);
	lcd_data(0x2A);
	lcd_data(0x28);
	lcd_data(0x26);
	lcd_data(0x2E);
	lcd_data(0x3A);
	lcd_data(0x00);
	lcd_data(0x01);
	lcd_data(0x03);
	lcd_data(0x13);

	lcd_cmd(0xE1);
	lcd_data(0x04);
	lcd_data(0x16);
	lcd_data(0x06);
	lcd_data(0x0D);
	lcd_data(0x2D);
	lcd_data(0x26);
	lcd_data(0x23);
	lcd_data(0x27);
	lcd_data(0x27);
	lcd_data(0x25);
	lcd_data(0x2D);
	lcd_data(0x3B);
	lcd_data(0x00);
	lcd_data(0x01);
	lcd_data(0x04);
	lcd_data(0x13);

	//------------------------------------End ST7735S Gamma Sequence-----------------------------//
	lcd_cmd(0x3A); //65k mode
	lcd_data(0x05);
	lcd_cmd(0x29);
	//Send all the commands
}

void LCD_drawPoint(uint16_t x, uint16_t y, uint16_t color)
{
	LCD_setAddress(x, y, x, y);
	lcd_color(color);
}

/**
	* @brief LCD显示一个1608字符函数
  * @param  x,y 字符的坐标
  *         color 16位RGB颜色
  * @retval None
  */
void LCD_showChar(uint16_t x, uint16_t y, uint8_t chr, uint16_t color)
{
	uint8_t i, j;
	uint8_t temp;
	chr = chr - ' ';
	LCD_setAddress(x, y, x+8-1, y+16-1);
	for(i=0; i<16; i++)
	{
		temp = asc2_1608[chr][i];
		for(j=0; j<8; j++)
		{
			if(temp&0x01)
			{
				LCD_drawPoint(x+j, y+i, color);
			}
			else
			{
				//LCD_drawPoint(x+j, y+i, BACK_COLOR);
			}
			temp >>= 1;
		}
	}
}

/**
	* @brief LCD显示字串函数
  * @param  x,y 字符的坐标
  *         *p 字符串
  *         color 16位RGB颜色
  * @retval None
  */
void LCD_showString(uint16_t x, uint16_t y, char *p, uint16_t color)
{
	while(*p!='\0')
	{
//		if(x>LCD_W-16)
//		{
//			x=0;
//			y+=16;
//		}
//		if(y>LCD_H-16)
//		{
//			y=x=0;
//			LCD_clear(BACK_COLOR);
//		}
		LCD_showChar(x, y, *p, color);
		x+=8;
		p++;
	}
}

/**
  * @brief LCD显示整数函数
  * @param  ,x,y 字符的坐标
  *         num 整数
  * 		len 显示为几位
  *         color 16位RGB颜色
  * @retval None
  */
void LCD_ShowIntNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint16_t color)
{
	uint8_t pChar[11] = {0};
	uint8_t i = 0;

	while(num > 0)
	{
		pChar[i++] = num % 10 + '0';
		num /= 10;
	}

	if(i < len)
	{
		i = len;
	}

	for(uint8_t k = i; k > 0; k--)
	{
		if(x > LCD_W - 8)
		{
			x = 0;
			y += 16;
		}

		if(y > LCD_H - 16)
		{
			x = 0;
			y = 0;
		}

		LCD_showChar(x, y, pChar[k - 1], color);
		x += 8;
	}
}

//通过字模显示一个汉字；
void Show_Dis_Chinese(uint16_t x, uint16_t y, uint8_t *ptm, uint16_t color)
{
	uint8_t temp, t1, t;
	uint16_t x0 = x;
	uint8_t csize = 32;//字节数；

	for(t = 0; t < csize; t++)
	{
		temp = ptm[t];

		for(t1 = 0; t1 < 8; t1++)
		{
			if(temp & 0x80)
			{
				LCD_drawPoint(x, y, color);
			}
			temp <<= 1;
			x++;
			if(x >= LCD_W)
			{
				return;
			}
			if((x - x0) == 16)
			{
				x = x0;
				y++;
				if(y >= LCD_H)
				{
					return;
				}
				break;
			}
		}
	}
}
/**
  * @brief LCD显示中文
  * @param  ,x,y 字符的坐标
  *         num 整数
  * 		len 显示为几位
  *         color 16位RGB颜色
  * @retval None
  */
void LCD_ShowChinese(uint16_t x, uint16_t y, uint8_t pxchar1, uint8_t pxchar2, uint16_t color, FILE * fp)
{
	uint8_t  char_L = 0, char_H = 0;
	uint32_t offset = 0;
	uint8_t chinese_char[32] = {0};

	if((pxchar1 & 0x80) && (pxchar2 & 0x80))
	{
		char_H = pxchar1 - 0xA0;
		char_L = pxchar2 - 0xA0;
		offset = ((char_H - 1) * 94 + (char_L - 1)) * 32;

		fseek(fp, offset, SEEK_SET);
		fread(chinese_char, 1, 32, fp);

		Show_Dis_Chinese(x, y, chinese_char, color);
	}
	else
	{
		return;
	}
}

void LCD_ShowChinese_C(uint16_t x, uint16_t y, uint8_t pxchar1, uint8_t pxchar2, uint16_t color)
{
	uint8_t  char_L = 0, char_H = 0;
	uint32_t offset = 0;
	uint8_t chinese_char[32] = {0};

	if((pxchar1 & 0x80) && (pxchar2 & 0x80))
	{
		char_H = pxchar1 - 0xA0;
		char_L = pxchar2 - 0xA0;
		offset = ((char_H - 1) * 94 + (char_L - 1)) * 32;

		const char *file_hello = MOUNT_POINT"/HZK16.txt";
		FILE * ftp = fopen(file_hello, "rb");
		fseek(ftp, offset, SEEK_SET);
		fread(chinese_char, 1, 32, ftp);

		Show_Dis_Chinese(x, y, chinese_char, color);

		fclose(ftp);
	}
	else
	{
		return;
	}
}

//中英文混合显示；
void Display_CE(uint16_t xes, uint16_t yes, char * Str, uint16_t color)
{
	char *ch_str;
	uint8_t adat[32] = {0};

	uint8_t wk_ucTem = 0;
	uint8_t wk_ucKem = 0;
	uint32_t wk_uLOffset = 0;

	FILE *ftp = NULL;
	uint8_t FTP_ucStr = 0;

	uint8_t i = 0;
	uint8_t len = 0;

	uint16_t ex = xes;
	uint16_t ey = yes;
	uint16_t x0 = xes;

	if(strlen(Str) > (100 - 1))
	{
		printf("string size too long");
	}

	utf82gbk(&ch_str, Str, strlen(Str));

	len = strlen(ch_str);

	for(i = 0; i < len; i++)
	{
		if((ex + 16) >= 240)
		{
			ex = x0;
			ey += 16;
		}

		if((ey + 16) >= 320)
		{
			return;
		}

		if(ch_str[i] & 0x80)//判断是否有中文；
		{
			if(FTP_ucStr == 0)
			{
				const char *file_hello = MOUNT_POINT"/HZK16.txt";
				ftp = fopen(file_hello, "rb");

				FTP_ucStr = 1;
			}

			wk_ucTem = (ch_str[i]) - 160;		//区码;
			wk_ucKem = (ch_str[i + 1]) - 160;		//位码;
			wk_uLOffset = ((wk_ucTem - 1) * 94 + (wk_ucKem - 1)) * 32; 	//偏移量;

			fseek(ftp, wk_uLOffset, SEEK_SET);
			fread(adat, 1, 32, ftp);

			Show_Dis_Chinese(ex, ey, adat, color);

			ex += 16;
			i++;
		}
		else//英文；
		{
			LCD_showChar(ex, ey, ch_str[i], color);
			ex += 8;
		}
	}

	if(FTP_ucStr != 0)
	{
		fclose(ftp);
	}
}


/****************************************************************************************************
 * 下面的函数是图片显示，显示图标不影响背景；
 ***************************************************************************************************/
//显示背景图片；
void LCD_Display(uint8_t xes, uint8_t yes, const uint8_t *pic)
{
	int len = 0;
	uint16_t width 	= ((uint16_t)pic[2]) << 8 | pic[3];
	uint16_t height 	= ((uint16_t)pic[4]) << 8 | pic[5];
	LCD_setAddress(xes, yes, xes + width - 1, yes + height - 1);

	len = width * height * 2;

	if(len <= SPI_MAX_NUM)
	{
		lcd_long_data(pic + 8, width * height);
	}
	else if(len > SPI_MAX_NUM)
	{
		int num = len /(1024 * 10);
		int tum = len %(1024 * 10);
		for(int j = 0; j < num; j++)
		{
			lcd_long_data(pic + 8 + j * SPI_MAX_NUM, SPI_MAX_NUM);
		}

		if(tum != 0)
		{
			lcd_long_data(pic + 8 + num * SPI_MAX_NUM, tum);
		}
	}

//	for(int i = 0; i < height; i++)
//	{
//		for(int j = 0; j < width; j++)
//		{
//			lcd_color(((uint16_t)(pic[8 + (i * width + j) * 2]) << 8) | pic[8 + (i * width + j) * 2 + 1]);
//		}
//	}
}

void LCD_clear_nDisplay(uint8_t xes, uint8_t yes, const uint8_t *pic)//清除n行背景；清除半个屏幕，位置自定义；
{
	int len = 0;
	uint16_t width 		= LCD_W;
	uint16_t height 	= LCD_H - yes;
	LCD_setAddress(xes, yes, xes + width - 1, yes + height - 1);

	len = width * height * 2;

	if(len <= SPI_MAX_NUM)
	{
		lcd_long_data(pic + 8 + LCD_W * yes * 2, len);
	}
	else if(len > SPI_MAX_NUM)
	{
		int num = len /(1024 * 10);
		int tum = len %(1024 * 10);
		for(int j = 0; j < num; j++)
		{
			lcd_long_data(pic + 8 + LCD_W * yes * 2 + j * SPI_MAX_NUM, SPI_MAX_NUM);
		}

		if(tum != 0)
		{
			lcd_long_data(pic + 8 +LCD_W * yes * 2 + num * SPI_MAX_NUM, tum);
		}
	}

//	for(int i = 0; i < height; i++)
//	{
//		for(int j = 0; j < width; j++)
//		{
//			lcd_color(((uint16_t)(pic[8 + (i * width + j) * 2]) << 8) | pic[8 + (i * width + j) * 2 + 1]);
//		}
//	}
}

//显示天气局部图；
void LCD_Display_Icon(uint8_t xes, uint8_t yes, uint8_t *pic, const uint8_t *back)
{
	//uint16_t color = 0;
	int len = 0;
	uint16_t width 	= ((uint16_t)pic[2]) << 8 | pic[3];
	uint16_t height 	= ((uint16_t)pic[4]) << 8 | pic[5];
	LCD_setAddress(xes, yes, xes + width - 1, yes + height - 1);
	len = width * height * 2;

	for(int i = 0; i < height; i++)
	{
		for(int j = 0; j < width; j++)
		{
//			color = ((uint16_t)(pic[8 + (i * width + j) * 2]) << 8) | pic[8 + (i * width + j) * 2 + 1];

			if(pic[8 + (i * width + j) * 2] == 0xFF && pic[8 + (i * width + j) * 2 + 1] == 0xFF)
			{
				pic[8 + (i * width + j) * 2] = back[8 + ((i + yes) * 128 + j + xes) * 2];
				pic[8 + (i * width + j) * 2 + 1] = back[8 + ((i + yes) * 128 + j + xes) * 2 + 1];
			}

//			if(color == 0xFFFF)
//			{
//				color = ((uint16_t)(back[8 + ((i + yes) * 128 + j + xes) * 2]) << 8) | back[8 + ((i + yes) * 128 + j + xes) * 2 + 1];
//			}
//			lcd_color(color);
		}
	}

	if(len <= SPI_MAX_NUM)
	{
		lcd_long_data(pic + 8, width * height);
	}
	else if(len > SPI_MAX_NUM)
	{
		int num = len /(1024 * 10);
		int tum = len %(1024 * 10);
		for(int j = 0; j < num; j++)
		{
			lcd_long_data(pic + 8 + j * SPI_MAX_NUM, SPI_MAX_NUM);
		}

		if(tum != 0)
		{
			lcd_long_data(pic + 8 + num * SPI_MAX_NUM, tum);
		}
	}


//	for(int i = 0; i < height; i++)
//	{
//		for(int j = 0; j < width; j++)
//		{
//			color = ((uint16_t)(pic[8 + (i * width + j) * 2]) << 8) | pic[8 + (i * width + j) * 2 + 1];
//			if(color == 0xFFFF)
//			{
//				color = ((uint16_t)(back[8 + ((i + yes) * 128 + j + xes) * 2]) << 8) | back[8 + ((i + yes) * 128 + j + xes) * 2 + 1];
//			}
////			else
////			{
////				color = YELLOW;
////			}
//
//			lcd_color(color);
//		}
//	}
}

//显示天气局部图；
void LCD_Display_Icon_cen(uint8_t xes, uint8_t yes, uint8_t *pic, const uint8_t *back, uint16_t * z_xes, uint16_t * z_yes)
{
	int W_f = 0;
	int H_f = 0;

	int len = 0;
	uint16_t width 	= ((uint16_t)pic[2]) << 8 | pic[3];
	uint16_t height 	= ((uint16_t)pic[4]) << 8 | pic[5];
	LCD_setAddress(xes, yes, xes + 120 - 1, yes + 120 - 1);
	len = width * height * 2;

	W_f = (120 - width) / 2;
	H_f = (120 - height) / 2;

	//*z_yes = yes + H_f + height;
	*z_yes = yes + 120;
	*z_xes = xes + W_f + width / 2;

	for(int i = 0; i < height; i++)//将白色替换为背景色；
	{
		for(int j = 0; j < width; j++)
		{
			if(pic[8 + (i * width + j) * 2] == 0xFF && pic[8 + (i * width + j) * 2 + 1] == 0xFF)
			{
				pic[8 + (i * width + j) * 2] = back[8 + ((i + yes + H_f) * LCD_W + j + xes + W_f) * 2];
				pic[8 + (i * width + j) * 2 + 1] = back[8 + ((i + yes+ H_f) * LCD_W + j + xes+ W_f) * 2 + 1];
			}

		}
	}

	for(int i = 0; i < 120; i++)//图片居中处理；
	{
		for(int j = 0; j < 120; j++)
		{
			if(i >= H_f && i < (H_f + height) && j >= W_f && j < (W_f + width))
			{
				bmp53[(i * 120 + j) * 2]     = pic[8 + ((i-H_f) * width + (j - W_f)) * 2];
				bmp53[(i * 120 + j) * 2 + 1] = pic[8 + ((i-H_f) * width + (j - W_f)) * 2 + 1];
			}
			else
			{
				bmp53[(i * 120 + j) * 2] = back[8 + ((i + yes) * LCD_W + j + xes) * 2];
				bmp53[(i * 120 + j) * 2 + 1] = back[8 + ((i + yes) * LCD_W + j + xes) * 2 + 1];
			}
		}
	}

	len = 120 * 120 *2;

	if(len <= SPI_MAX_NUM)//发送到屏幕；
	{
		lcd_long_data(bmp53, len);
	}
	else if(len > SPI_MAX_NUM)
	{
		int num = len /(1024 * 10);
		int tum = len %(1024 * 10);

		for(int j = 0; j < num; j++)
		{
			lcd_long_data(bmp53 + j * SPI_MAX_NUM, SPI_MAX_NUM);
		}

		if(tum != 0)
		{
			lcd_long_data(bmp53 + num * SPI_MAX_NUM, tum);
		}
	}
}


void LCD_Display_bmp(uint8_t xes, uint8_t yes, char * pname, const uint8_t *back, uint16_t * z_xes, uint16_t * z_yes)
{
	FILE * fp = NULL;
	uint32_t num_Len = 0;
	char fname[50] = {0};
	strcpy(fname, MOUNT_POINT);
	strcpy(fname + strlen((char *)MOUNT_POINT), pname);
	//const char *file_name = MOUNT_POINT"/bmp3.txt";

	fp = fopen(fname, "rb");
	if(fp == NULL)
	{
		printf("fp == NULL");
	}
	fseek(fp, 0, SEEK_END);

	num_Len = ftell(fp);//求字节长度；全文字节数；
	fseek(fp, 0, SEEK_SET);
	fread(bmp52, 1, num_Len, fp);
	//LCD_Display_Icon(xes, yes, bmp52, back);
	LCD_Display_Icon_cen(xes, yes, bmp52, back, z_xes, z_yes);

	fclose(fp);
}

void LCD_Display_52(uint8_t xes, uint8_t yes, const uint8_t *back)
{
	FILE * fp = NULL;
	uint32_t Hex_Full_Len = 0;
	const char *file_name = MOUNT_POINT"/bmp5_2.txt";

	fp = fopen(file_name, "rb");
	if(fp != NULL)
	{
		printf("fp ！= NULL");
	}
	fseek(fp, 0, SEEK_END);

	Hex_Full_Len = ftell(fp);//求字节长度；全文字节数；
	fseek(fp, 0, SEEK_SET);
	fread(bmp52, 1, Hex_Full_Len, fp);
	LCD_Display_Icon(xes, yes, bmp52, back);
	fclose(fp);
}

/****************************************************************************************************
 * 下面函数是带背景的字体显示；
 ***************************************************************************************************/

//显示中英文字体，字体改变背景不变；
void LCD_showChar_bc(uint16_t x, uint16_t y, uint8_t chr, uint16_t color, const uint8_t * back)
{
	uint8_t i, j;
	uint8_t temp;
	chr = chr - ' ';
	LCD_setAddress(x, y, x+8-1, y+16-1);
	for(i=0; i<16; i++)
	{
		temp = asc2_1608[chr][i];
		for(j=0; j<8; j++)
		{
			if(temp&0x01)
			{
				LCD_drawPoint(x+j, y+i, color);
			}
			else
			{
				LCD_drawPoint(x+j, y+i, (uint16_t)(back[8 + ((i + y) * LCD_W + j + x) * 2]) << 8 | (back[8 + ((i + y) * LCD_W + j + x) * 2 + 1]));
				//LCD_drawPoint(x+j, y+i, BACK_COLOR);
			}
			temp >>= 1;
		}
	}
}

void LCD_showString_bc(uint16_t x, uint16_t y, char *p, uint16_t color, const uint8_t * back)
{
	while(*p!='\0')
	{
		if(x>LCD_W-16)
		{
			x=0;
			y+=16;
		}
		if(y>LCD_H-16)
		{
			y=x=0;
			LCD_clear(BACK_COLOR);
		}
		LCD_showChar_bc(x, y, *p, color, back);
		x+=8;
		p++;
	}
}

//通过字模显示一个汉字；
void Show_Dis_Chinese_bc(uint16_t x, uint16_t y, uint8_t *ptm, uint16_t color, const uint8_t * back)
{
	uint8_t temp, t1, t;
	uint16_t x0 = x;
	uint8_t csize = 32;//字节数；

	for(t = 0; t < csize; t++)
	{
		temp = ptm[t];

		for(t1 = 0; t1 < 8; t1++)
		{
			if(temp & 0x80)
			{
				LCD_drawPoint(x, y, color);
			}
			else
			{
				LCD_drawPoint(x, y, (uint16_t)(back[8 + (y * LCD_W + x) * 2]) << 8 | (back[8 + (y * LCD_W + x) * 2 + 1]));
			}
			temp <<= 1;
			x++;
			if(x >= LCD_W)
			{
				return;
			}
			if((x - x0) == 16)
			{
				x = x0;
				y++;
				if(y >= LCD_H)
				{
					return;
				}
				break;
			}
		}
	}
}

static INLINE__ void* gbk_malloc(uint32_t size)
{
    void *p = malloc(size);

    if (p != NULL)
    {
        return p;
    }

    printf("[%s]xmalloc fail!!\n", __FUNCTION__);
//    exit(EXIT_FAILURE);
    exit(-1);

}

int utf8_gbk(char **ptr, void *pin_buf, s32 in_len)
{
	if ((ptr == NULL) || (pin_buf == NULL) || (in_len < 0))
	{
		return -1;
	}

	*ptr = NULL;

	u8 *ps = (u8 *)gbk_malloc((in_len*2)/3 + 4);
	u8 *pout = ps;
	u8 *pin = (u8 *)pin_buf;
	u8 *pend = pin + in_len;

	while (pin < pend)
	{
		u8 h = *pin++;

		if (!(h & 0x80))
		{
			*pout++ = h;
			continue;
		}

		if ((pin + 2) > pend)
		{
			free(ps);
			return -1;
		}

		u8 mid = pin[0];
		u8 end = pin[1];

		if (h < 0xe4 || h > 0xe9 || mid < 0x80 || end < 0x80 || mid > 0xbf || end > 0xbf
				|| (h == 0xe9 && mid > 0xbe) || (h == 0xe9 && mid == 0xbe && end > 0xa5)
				|| (h == 0xe4 && mid < 0xb8) || (h == 0xe9 && mid == 0xb8 && end < 0x80))
		{
			free(ps);
			return -1;
		}

		int offset = (h - 0xe4) * 64 * 64 + (mid - 0xb8) * 64 + end - 0x80;

		//u16 uniode = ((u16)h << 12) | ((u16)(mid & 0x3f) << 6) | (end & 0x3f);
		//u16 gbk = uni2gbk2(uniode);

//		if (gbk == 0)
//		{
//			free(ps);
//			return -1;
//		}

		pin += 2;
		*pout++ = gbk[offset][0];
		*pout++ = gbk[offset][1];
	}

	*ptr = (char*)ps;
	*pout = 0;

	return pout - ps;
}

void Display_CE_bc(uint16_t xes, uint16_t yes, char * Str, uint16_t color, const uint8_t * back)
{
	char *ch_str = NULL;
	uint8_t adat[32] = {0};

	uint8_t wk_ucTem = 0;
	uint8_t wk_ucKem = 0;
	uint32_t wk_uLOffset = 0;

	FILE *ftp = NULL;
	uint8_t FTP_ucStr = 0;

	uint8_t i = 0;
	uint16_t len = 0;
	uint16_t len0 = 0;

	uint16_t ex = xes;
	uint16_t ey = yes;
	//uint16_t x0 = xes;

	int j = 0;
	while(Str[j] != 0)
	{
		if (Str[j] <= 0x7F) //then ASCII 占用1个字节
		{
			len0 += 1;
			j += 1;
		}
		else if (Str[j] >= 0xC0 && Str[j] <= 0xDF) // then 首字节   UTF-8 占用2个字节
		{
			len0 += 2;
			j += 2;
		}
		else if (Str[j] >= 0xE0 && Str[j] <= 0xEF) // then 首字节   UTF-8 占用3个字节
		{
			len0 += 3;
			j += 3;
		}
	}
	//printf("len0 = %d\n", len0);
	if(len0 > (100 - 1))
	{
		printf("string size too long\n");
		printf("len0 = %d\n", len0);
	}

	//utf82gbk(&ch_str, Str, len0);

	utf8_gbk(&ch_str, Str, len0);
	len = strlen(ch_str);

	//printf("len = %d\n", len);

	for(i = 0; i < len; i++)
	{
//		if((ex + 8) >= 240)
//		{
//			ex = x0;
//			ey += 16;
//		}
//
//		if((ey + 8) >= 320)
//		{
//			return;
//		}

		if(ch_str[i] & 0x80)//判断是否有中文；
		{
			if(FTP_ucStr == 0)
			{
				const char *file_hello = MOUNT_POINT"/HZK16.txt";
				ftp = fopen(file_hello, "rb");

				FTP_ucStr = 1;
			}

			wk_ucTem = (ch_str[i]) - 160;		//区码;
			wk_ucKem = (ch_str[i + 1]) - 160;		//位码;
			wk_uLOffset = ((wk_ucTem - 1) * 94 + (wk_ucKem - 1)) * 32; 	//偏移量;

			fseek(ftp, wk_uLOffset, SEEK_SET);
			fread(adat, 1, 32, ftp);

			Show_Dis_Chinese_bc(ex, ey, adat, color, back);

			ex += 16;
			i++;
		}
		else//英文；
		{
			LCD_showChar_bc(ex, ey, ch_str[i], color, back);
			ex += 8;
		}
	}

	if(FTP_ucStr != 0)
	{
		fclose(ftp);
	}
	free(ch_str);
}


void LCD_ShowChinese_C_bc(uint16_t x, uint16_t y, uint8_t pxchar1, uint8_t pxchar2, uint16_t color, const uint8_t * back)
{
	uint8_t  char_L = 0, char_H = 0;
	uint32_t offset = 0;
	uint8_t chinese_char[32] = {0};

	if((pxchar1 & 0x80) && (pxchar2 & 0x80))
	{
		char_H = pxchar1 - 0xA0;
		char_L = pxchar2 - 0xA0;
		offset = ((char_H - 1) * 94 + (char_L - 1)) * 32;

		const char *file_hello = MOUNT_POINT"/HZK16.txt";
		FILE * ftp = fopen(file_hello, "rb");
		fseek(ftp, offset, SEEK_SET);
		fread(chinese_char, 1, 32, ftp);

		Show_Dis_Chinese_bc(x, y, chinese_char, color, back);

		fclose(ftp);
	}
	else
	{
		return;
	}
}

//显示中英文字体，字体改变背景不变；
void LCD_showChar48_bc(uint16_t x, uint16_t y, uint8_t chr, uint16_t color, const uint8_t * back)
{
	uint16_t i, j, k;
	uint8_t temp;
	chr = chr - '0';
	if(chr > 9)
	{
		return;
	}

	LCD_setAddress(x, y, x+24-1, y+48-1);
	for(i=0; i<48; i++)
	{
		for(k = 0; k < 3; k++)
		{
			temp = asc2_4824[chr][i * 3 + k];
			for(j = 0; j < 8; j++)
			{
				if(temp & 0x80)
				{
					LCD_drawPoint(x+j + k * 8, y+i, color);
				}
				else
				{
					LCD_drawPoint(x+j + k * 8, y+i, (uint16_t)(back[8 + ((i + y) * LCD_W + j + x + k * 8) * 2]) << 8 | (back[8 + ((i + y) * LCD_W + j + x + k * 8) * 2 + 1]));
					//LCD_drawPoint(x+j, y+i, BACK_COLOR);
				}
				temp <<= 1;
			}
		}

	}
}

void LCD_showStr48_bc(uint16_t x, uint16_t y, char * Str, uint16_t color, const uint8_t * back)
{
	while(*Str!='\0')
	{
		if(x>LCD_W-24)
		{
			x=0;
			y+=48;
		}
		if(y>LCD_H-48)
		{
			y=x=0;
			LCD_clear(BACK_COLOR);
		}
		LCD_showChar48_bc(x, y, *Str, color, back);
		x+=24;
		Str++;
	}
}

//画斜线；
void LCD_Draw_Line(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t color)
{
	uint16_t x1, x2, y1, y2;
	uint16_t i, j, k;
	uint16_t ys, ye;
	float FL_fShowXy;
	uint16_t FL_usBase_Y;
	float FL_fshow_y;

	float FL_f_x;
	float FL_f_y;

	if(sx > ex)
	{
		x1 = ex;
		x2 = sx;
		y1 = ey;
		y2 = sy;
	}
	else
	{
		x1 = sx;
		x2 = ex;
		y1 = sy;
		y2 = ey;
	}

	if(x1 == x2)
	{
		if(y1 > y2)
		{
			ys = y2;
			ye = y1;
		}
		else
		{
			ys = y1;
			ye = y2;
		}

		for(i = ys; i <= ye; i++)
		{
			LCD_drawPoint(x1, i, color);
		}

	}
	else if(y1 == y2)
	{
//		LCD_SetCursor(x1, y1);
//		LCD_WriteRAM_Start();
		LCD_setAddress(x1, y1, x2, y1);
		for(j = 0; j < x2 - x1 + 1; j++)
		{

			lcd_color(color);
		}
	}
	else
	{
		FL_f_x = (float)(y2 - y1 + 1);
		FL_f_y = (float)(x2 - x1 + 1);
		FL_fShowXy = FL_f_x / FL_f_y;//计算斜率；

		if((y2 - y1) >= 0)
		{
			FL_f_x = (float)(y2 - y1 + 1);
			FL_f_y = (float)(x2 - x1 + 1);
			FL_fShowXy = FL_f_x / FL_f_y;//计算斜率；

			if(FL_fShowXy <= 1)
			{
				FL_usBase_Y = 0;
				for(k = 1; k <= x2 - x1 + 1; k++)
				{
					FL_fshow_y = FL_fShowXy * k;
					if(FL_fshow_y > FL_usBase_Y + 1)
					{
						if((FL_fshow_y - FL_usBase_Y - 1) > ((1 * FL_fShowXy) / 2))
						{
							LCD_drawPoint(x1 + k - 1, y1 + FL_usBase_Y + 1, color);
						}
						else
						{
							LCD_drawPoint(x1 + k - 1, y1 + FL_usBase_Y, color);
						}
						FL_usBase_Y ++;
					}
					else
					{
						LCD_drawPoint(x1 + k - 1, y1 + FL_usBase_Y, color);
					}

				}
			}
			else if(FL_fShowXy > 1)
			{
				FL_usBase_Y = 0;
				for(k = 1; k <= x2 - x1 + 1; k++)
				{
//					m = 0;
					FL_fshow_y = FL_fShowXy * k;
					while((FL_fshow_y > FL_usBase_Y + 1))
					{
						LCD_drawPoint(x1 + k - 1, y1 + FL_usBase_Y, color);
						FL_usBase_Y ++;
					}

					if((FL_fshow_y - FL_usBase_Y) > 0.5)
					{
						LCD_drawPoint(x1 + k - 1, y1 + FL_usBase_Y, color);
						FL_usBase_Y ++;
					}
				}
			}
		}
		else
		{
			FL_f_x = (float)(y2 - y1 - 1);
			FL_f_y = (float)(x2 - x1 + 1);
			FL_fShowXy = FL_f_x / FL_f_y;//计算斜率；

			if(FL_fShowXy >= -1)
			{
				FL_fShowXy = -FL_fShowXy;
				FL_usBase_Y = 0;
				for(k = 1; k <= x2 - x1 + 1; k++)
				{
					FL_fshow_y = FL_fShowXy * k;
					if(FL_fshow_y > FL_usBase_Y + 1)
					{
						if((FL_fshow_y - FL_usBase_Y - 1) > ((1 * FL_fShowXy) / 2))
						{
							LCD_drawPoint(x1 + k - 1, y1 - FL_usBase_Y - 1, color);
						}
						else
						{
							LCD_drawPoint(x1 + k - 1, y1 - FL_usBase_Y, color);
						}
						FL_usBase_Y ++;
					}
					else
					{
						LCD_drawPoint(x1 + k - 1, y1 - FL_usBase_Y, color);
					}

				}
			}
			else if(FL_fShowXy < -1)
			{
				FL_fShowXy = -FL_fShowXy;
				FL_usBase_Y = 0;
				for(k = 1; k <= x2 - x1 + 1; k++)
				{
					//m = 0;
					FL_fshow_y = FL_fShowXy * k;
					while((FL_fshow_y > FL_usBase_Y + 1) && ((y1 - FL_usBase_Y) >= 0))
					{
						LCD_drawPoint(x1 + k - 1, y1 - FL_usBase_Y, color);
						FL_usBase_Y ++;
					}

					if((FL_fshow_y - FL_usBase_Y) > 0.5)
					{
						LCD_drawPoint(x1 + k - 1, y1 - FL_usBase_Y, color);
						FL_usBase_Y ++;
					}
				}
			}
		}
	}
}

//画圆；
void LCD_Draw_Circle(uint16_t xs, uint16_t ys, uint16_t RS, uint16_t color)
{
	uint16_t x1, y1, r1;
	uint16_t Base_Y;
	uint16_t Base_X;
	uint16_t i;

	x1 = xs;
	y1 = ys;
	r1 = RS;

	Base_X = 0;
	Base_Y = r1;

	while((2 * Base_X * Base_X) < (r1 * r1))
	{
		Base_X++;
	}

	for(i = 0; i < Base_X; i++)
	{
		if(((i + 0.5) * (i + 0.5) + (Base_Y - 1) * (Base_Y - 1)) >= (r1 * r1))
		{
			Base_Y--;
		}
		LCD_drawPoint(x1 + i, y1 + Base_Y, color);//一个1/4之一圆；
		LCD_drawPoint(x1 + Base_Y, y1 + i, color);

		LCD_drawPoint(x1 + i, y1 - 1 - Base_Y, color);
		LCD_drawPoint(x1 + Base_Y, y1 - 1 - i, color);

		LCD_drawPoint(x1 - 1 - i, y1 + Base_Y, color);//一个1/4之一圆；
		LCD_drawPoint(x1 - 1 - Base_Y, y1 + i, color);

		LCD_drawPoint(x1 - 1 - i, y1 - 1 - Base_Y, color);
		LCD_drawPoint(x1 - 1 - Base_Y, y1 - 1 - i, color);
	}
}

//画实体圆；
void LCD_DrawFullCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius, uint16_t color)
{
	uint16_t x, y, r = Radius;

	for(y = Ypos - r; y < Ypos + r; y++)
	{
		for(x = Xpos - r; x < Xpos+r; x++)
		{
			if(((x-Xpos)*(x-Xpos)+(y-Ypos)*(y-Ypos)) <= r*r)
			{
				//putpixel(x,y);
				LCD_drawPoint(x, y, color);
			}
		}
	}
}

