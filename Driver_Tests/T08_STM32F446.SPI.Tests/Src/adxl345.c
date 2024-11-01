#include "adxl345.h"

#define		MULTI_BYTE_EN		0x40
#define		READ_OPERATION		0x80

SPIObject spiObj =
{
	.chipSelectPinNumber = 3,
	.chipSelectPinPort = GPIOC,
	.spiNumber = SPI_OBJ_3,
	.baudRatePrescaler = 2
};

//SPIObject spiObj =
//{
//	.chipSelectPinNumber = 9,
//	.chipSelectPinPort = GPIOA,		// works !!
//	.spiNumber = 1,
//  .baudRatePrescaler = 2
//};

void adxl_read(uint8_t address, uint8_t* rxData, uint8_t size)
{
	// set read operation
	address |= READ_OPERATION;

	// enable multibyte, place address into buffer
	address |= MULTI_BYTE_EN;

	// pull cs line low to enable slave
	cs_enable(&spiObj);

	// send address
	spi_transmit(&spiObj, &address, 1);

	// receive 6 bytes
	spi_receive(&spiObj, rxData, size);

	// pull cs line high to disable slave
	cs_disable(&spiObj);
}

void adxl_write(uint8_t address, uint8_t value)
{
	uint8_t data[2];

	// enable multibyte, place address into buffer
	data[0] = address|MULTI_BYTE_EN;

	// place data into buffer
	data[1] = value;

	// pull cs line low to enable slave
	cs_enable(&spiObj);

	// transmit data and address
	spi_transmit(&spiObj, data, 2);

	// pull cs line high to disable slave
	cs_disable(&spiObj);
}

void adxl_init()
{
	// enable SPI GPIO
	//spi_gpio_init();

	// config SPI
	//spi1_config();

	spi_init(&spiObj);

	// set data format range to +-4g
	adxl_write(DATA_FORMAT_R, FOUR_G);

	// reset all bits
	adxl_write(POWER_CTRL_R, RESET);

	// configure power control measure bit
	adxl_write(POWER_CTRL_R, SET_MEASURE_B);
}
