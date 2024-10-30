#ifndef SPI_H_
#define SPI_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f4xx.h"
#include "stdint.h"

#define		SPI_OBJ_1		1
#define		SPI_OBJ_2		2
#define		SPI_OBJ_3		3

#define		SPI_OK							0
#define		SPI_ERROR_INVALID_ARGUMENT		(-22)

typedef int spi_result_t;

typedef struct
{
	GPIO_TypeDef * chipSelectPinPort;
	uint32_t chipSelectPinNumber;
	uint32_t spiNumber;
	uint32_t baudRatePrescaler;

}SPIObject;

spi_result_t spi_init(SPIObject * self);
spi_result_t spi_deinit(SPIObject * self);
spi_result_t spi_transmit(SPIObject * self, uint8_t *data, uint32_t size);
spi_result_t spi_receive(SPIObject * self, uint8_t *data, uint32_t size);
spi_result_t cs_enable(SPIObject * self);
spi_result_t cs_disable(SPIObject * self);

/*
 * Pins for SPI1: PA5 - SCK  / PA6 - MISO  / PA7 - MOSI  - AF5
 * Pins for SPI2: PB13 - SCK / PB14 - MISO / PB15 - MOSI - AF5
 * Pins for SPI3: PC10 - SCK / PC11 - MISO / PC12 - MOSI - AF6
 * */

#ifdef __cplusplus
 } // extern "C"
#endif

#endif /* SPI_H_ */
