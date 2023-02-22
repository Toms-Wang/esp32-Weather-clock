#include "lcd.h"
#include "font.h"
#include "hzk.h"

uint8_t PARALLEL_LINES = 16;
spi_device_handle_t spi;
uint16_t BACK_COLOR = WHITE;//默认背景色；
uint8_t color_t[LCD_W * 2] = {0};

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
		.max_transfer_sz=PARALLEL_LINES*240*2+8
	};

	spi_device_interface_config_t devcfg =
	{
		.clock_speed_hz=16*1000*1000,
		.mode=0,                                //SPI mode 0
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

void lcd_long_data(uint8_t *pdata, uint16_t len)
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
	lcd_color(x1 + 2);
	lcd_color(x2 + 2);
	lcd_cmd(0x2b);
	lcd_color(y1 + 3);
	lcd_color(y2 + 3);
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
				LCD_drawPoint(x+j, y+i, BACK_COLOR);
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

