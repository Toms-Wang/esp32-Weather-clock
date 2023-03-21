#include "sd_spi.h"


static const char mount_point[] = MOUNT_POINT;
sdmmc_card_t *card;

void spi_SD_init(void)
{
	esp_err_t ret;
	sdmmc_host_t host = SDSPI_HOST_DEFAULT();
	host.slot = SD_HOST;

	spi_bus_config_t buscfg =
	{
		.miso_io_num=PIN_SD_MISO,
		.mosi_io_num=PIN_SD_MOSI,
		.sclk_io_num=PIN_SD_CLK,
		.quadwp_io_num=-1,
		.quadhd_io_num=-1,
		.max_transfer_sz=4 * 1024 * sizeof(uint8_t)
	};

	//Initialize the SPI bus
	ret=spi_bus_initialize(host.slot, &buscfg, SPI_DMA_CH_AUTO);
	if(!ret)
	{
		printf("spi bus init success!\n");
	}
	else
	{
		printf("spi bus init fail!\n");
		printf("ret = %x\n", ret);
	}
	ESP_ERROR_CHECK(ret);

	sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
	slot_config.gpio_cs = PIN_SD_CS;
	slot_config.host_id = host.slot;

	esp_vfs_fat_sdmmc_mount_config_t mount_config =
	{
			.format_if_mount_failed = false,
			.max_files = 5,
			.allocation_unit_size = 16 * 1024
	};

	ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
	if(!ret)
	{
		printf("file system mount success!\n");
		sdmmc_card_print_info(stdout, card);
	}
	else
	{
		printf("file system mount fail!\n");
		printf("ret = %x\n", ret);

	}

}

void SD_GPIO_Init(void)
{
	gpio_config_t io_conf = {};
	//disable interrupt
	io_conf.intr_type = GPIO_INTR_DISABLE;
	//set as output mode
	io_conf.mode = GPIO_MODE_OUTPUT;
	//bit mask of the pins that you want to set,e.g.21
	io_conf.pin_bit_mask = (1ULL << PIN_SD_CS);
	//disable pull-down mode
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	//disable pull-up mode
	io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
	//configure GPIO with the given settings
	gpio_config(&io_conf);

	//gpio_set_level(PIN_SD_CS, 0);
}

void SD_GPIO_Init_14(void)
{
	gpio_config_t io_conf = {};
	//disable interrupt
	io_conf.intr_type = GPIO_INTR_DISABLE;
	//set as output mode
	io_conf.mode = GPIO_MODE_OUTPUT;
	//bit mask of the pins that you want to set,e.g.21
	io_conf.pin_bit_mask = (1ULL << 14);
	//disable pull-down mode
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	//disable pull-up mode
	io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
	//configure GPIO with the given settings
	gpio_config(&io_conf);

	gpio_set_level(14, 0);
}
