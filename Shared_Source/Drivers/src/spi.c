#include "spi.h"
#include <common.h>

#define SPI1EN		(1U<<12)
#define	SPI2EN		(1U<<14)
#define	SPI3EN		(1U<<15)

#define	GPIOAEN		(1U<<0)
#define	GPIOBEN		(1U<<1)
#define	GPIOCEN		(1U<<2)

#define	SR_TXE		(1U<<1)
#define	SR_RXNE		(1U<<0)
#define	SR_BSY		(1U<<7)

static void spi_config(SPIObject * self);

static void init_chip_select_pin(SPIObject * self);
static void deinit_chip_select_pin(SPIObject * self);

static void init_gpio_for_spi1();
static void init_gpio_for_spi2();
static void init_gpio_for_spi3();

static void deinit_gpio_for_spi1();
static void deinit_gpio_for_spi2();
static void deinit_gpio_for_spi3();

static SPI_TypeDef * get_spi_module_from_number(uint32_t spi_num);
static spi_result_t validate_self(SPIObject * self);
static void set_baudrate_prescaler(SPIObject * self);

spi_result_t spi_init(SPIObject * self)
{
	if(validate_self(self) != SPI_OK)
	{
		return SPI_ERROR_INVALID_ARGUMENT;
	}

	if(self->spiNumber == SPI_OBJ_1)
	{
		init_gpio_for_spi1();
	}
	else if(self->spiNumber == SPI_OBJ_2)
	{
		init_gpio_for_spi2();
	}
	else if(self->spiNumber == SPI_OBJ_3)
	{
		init_gpio_for_spi3();
	}

	init_chip_select_pin(self);
	spi_config(self);

	return SPI_OK;
}

spi_result_t spi_deinit(SPIObject * self)
{
	if(validate_self(self) != SPI_OK)
	{
		return SPI_ERROR_INVALID_ARGUMENT;
	}

	SPI_TypeDef * spiModule = get_spi_module_from_number(self->spiNumber);
	spiModule->CR1 = 0;

	deinit_chip_select_pin(self);

	if(self->spiNumber == SPI_OBJ_1)
	{
		RCC->APB2ENR &= ~(SPI1EN);
		deinit_gpio_for_spi1();
	}
	else if(self->spiNumber == SPI_OBJ_2)
	{
		RCC->APB1ENR &= ~(SPI2EN);
		deinit_gpio_for_spi2();
	}
	else if(self->spiNumber == SPI_OBJ_3)
	{
		RCC->APB1ENR &= ~(SPI3EN);
		deinit_gpio_for_spi3();
	}
	return SPI_OK;
}


static SPI_TypeDef * get_spi_module_from_number(uint32_t spiNumber)
{
	switch(spiNumber)
	{
	case SPI_OBJ_1:
		return SPI1;
	case SPI_OBJ_2:
		return SPI2;
	case SPI_OBJ_3:
		return SPI3;
	default:
		return NULL;
	}
}

static void init_chip_select_pin(SPIObject * self)
{
	uint32_t firstModerBit = 2*self->chipSelectPinNumber;

	// config as output pin
	self->chipSelectPinPort->MODER |= (1U<<firstModerBit);
	self->chipSelectPinPort->MODER &= ~(1U<<(firstModerBit+1));
}

static void deinit_chip_select_pin(SPIObject * self)
{
	uint32_t firstModerBit = 2*self->chipSelectPinNumber;

	// config as input pin (reset state)
	self->chipSelectPinPort->MODER &= ~(1U<<firstModerBit);
	self->chipSelectPinPort->MODER &= ~(1U<<(firstModerBit+1));
}

static void init_gpio_for_spi1()
{
	// enable clock access to GPIOA
	RCC->AHB1ENR |= GPIOAEN;

	// set pins to alternate function

	// PA5
	GPIOA->MODER &= ~(1U<<10);
	GPIOA->MODER |= (1U<<11);

	// PA6
	GPIOA->MODER &= ~(1U<<12);
	GPIOA->MODER |= (1U<<13);

	// PA7
	GPIOA->MODER &= ~(1U<<14);
	GPIOA->MODER |= (1U<<15);

	// set pins to alternate function type AF05

	// PA5
	GPIOA->AFR[0] |= (1U<<20);
	GPIOA->AFR[0] &= ~(1U<<21);
	GPIOA->AFR[0] |= (1U<<22);
	GPIOA->AFR[0] &= ~(1U<<23);

	// PA6
	GPIOA->AFR[0] |= (1U<<24);
	GPIOA->AFR[0] &= ~(1U<<25);
	GPIOA->AFR[0] |= (1U<<26);
	GPIOA->AFR[0] &= ~(1U<<27);

	// PA7
	GPIOA->AFR[0] |= (1U<<28);
	GPIOA->AFR[0] &= ~(1U<<29);
	GPIOA->AFR[0] |= (1U<<30);
	GPIOA->AFR[0] &= ~(1U<<31);
}

static void deinit_gpio_for_spi1()
{
	// set pins to input (reset state)

	// PA5
	GPIOA->MODER &= ~(1U<<10);
	GPIOA->MODER &= ~(1U<<11);

	// PA6
	GPIOA->MODER &= ~(1U<<12);
	GPIOA->MODER &= ~(1U<<13);

	// PA7
	GPIOA->MODER &= ~(1U<<14);
	GPIOA->MODER &= ~(1U<<15);

	// reset alternate function type

	// PA5
	GPIOA->AFR[0] &= ~(1U<<20);
	GPIOA->AFR[0] &= ~(1U<<21);
	GPIOA->AFR[0] &= ~(1U<<22);
	GPIOA->AFR[0] &= ~(1U<<23);

	// PA6
	GPIOA->AFR[0] &= ~(1U<<24);
	GPIOA->AFR[0] &= ~(1U<<25);
	GPIOA->AFR[0] &= ~(1U<<26);
	GPIOA->AFR[0] &= ~(1U<<27);

	// PA7
	GPIOA->AFR[0] &= ~(1U<<28);
	GPIOA->AFR[0] &= ~(1U<<29);
	GPIOA->AFR[0] &= ~(1U<<30);
	GPIOA->AFR[0] &= ~(1U<<31);
}

static void init_gpio_for_spi2()
{
	// enable clock access to GPIOB
	RCC->AHB1ENR |= GPIOBEN;

	// set pins to alternate function

	// PB13
	GPIOB->MODER &= ~(1U<<26);
	GPIOB->MODER |= (1U<<27);

	// PB14
	GPIOB->MODER &= ~(1U<<28);
	GPIOB->MODER |= (1U<<29);

	// PB15
	GPIOB->MODER &= ~(1U<<30);
	GPIOB->MODER |= (1U<<31);

	// set pins to alternate function type AF05

	// PB13
	GPIOB->AFR[1] |= (1U<<20);
	GPIOB->AFR[1] &= ~(1U<<21);
	GPIOB->AFR[1] |= (1U<<22);
	GPIOB->AFR[1] &= ~(1U<<23);

	// PB14
	GPIOB->AFR[1] |= (1U<<24);
	GPIOB->AFR[1] &= ~(1U<<25);
	GPIOB->AFR[1] |= (1U<<26);
	GPIOB->AFR[1] &= ~(1U<<27);

	// PB15
	GPIOB->AFR[1] |= (1U<<28);
	GPIOB->AFR[1] &= ~(1U<<29);
	GPIOB->AFR[1] |= (1U<<30);
	GPIOB->AFR[1] &= ~(1U<<31);
}

static void deinit_gpio_for_spi2()
{
	// set pins to input (reset state)

	// PB13
	GPIOB->MODER &= ~(1U<<26);
	GPIOB->MODER &= ~(1U<<27);

	// PB14
	GPIOB->MODER &= ~(1U<<28);
	GPIOB->MODER &= ~(1U<<29);

	// PB15
	GPIOB->MODER &= ~(1U<<30);
	GPIOB->MODER &= ~(1U<<31);

	// reset alternate function register

	// PB13
	GPIOB->AFR[1] &= ~(1U<<20);
	GPIOB->AFR[1] &= ~(1U<<21);
	GPIOB->AFR[1] &= ~(1U<<22);
	GPIOB->AFR[1] &= ~(1U<<23);

	// PB14
	GPIOB->AFR[1] &= ~(1U<<24);
	GPIOB->AFR[1] &= ~(1U<<25);
	GPIOB->AFR[1] &= ~(1U<<26);
	GPIOB->AFR[1] &= ~(1U<<27);

	// PB15
	GPIOB->AFR[1] &= ~(1U<<28);
	GPIOB->AFR[1] &= ~(1U<<29);
	GPIOB->AFR[1] &= ~(1U<<30);
	GPIOB->AFR[1] &= ~(1U<<31);
}

static void init_gpio_for_spi3()
{
	// enable clock access to GPIOB
	RCC->AHB1ENR |= GPIOCEN;

	// set pins to alternate function

	// PC10
	GPIOC->MODER &= ~(1U<<20);
	GPIOC->MODER |= (1U<<21);

	// PC11
	GPIOC->MODER &= ~(1U<<22);
	GPIOC->MODER |= (1U<<23);

	// PC12
	GPIOC->MODER &= ~(1U<<24);
	GPIOC->MODER |= (1U<<25);

	// set pins to alternate function type AF06

	// PC10
	GPIOC->AFR[1] &= ~(1U<<8);
	GPIOC->AFR[1] |= (1U<<9);
	GPIOC->AFR[1] |= (1U<<10);
	GPIOC->AFR[1] &= ~(1U<<11);

	// PC11
	GPIOC->AFR[1] &= ~(1U<<12);
	GPIOC->AFR[1] |= (1U<<13);
	GPIOC->AFR[1] |= (1U<<14);
	GPIOC->AFR[1] &= ~(1U<<15);

	// PC12
	GPIOC->AFR[1] &= ~(1U<<16);
	GPIOC->AFR[1] |= (1U<<17);
	GPIOC->AFR[1] |= (1U<<18);
	GPIOC->AFR[1] &= ~(1U<<19);
}

static void deinit_gpio_for_spi3()
{
	// set pins to input (reset state)

	// PC10
	GPIOC->MODER &= ~(1U<<20);
	GPIOC->MODER &= ~(1U<<21);

	// PC11
	GPIOC->MODER &= ~(1U<<22);
	GPIOC->MODER &= ~(1U<<23);

	// PC12
	GPIOC->MODER &= ~(1U<<24);
	GPIOC->MODER &= ~(1U<<25);

	// reset alternate function type register

	// PC10
	GPIOC->AFR[1] &= ~(1U<<8);
	GPIOC->AFR[1] &= ~(1U<<9);
	GPIOC->AFR[1] &= ~(1U<<10);
	GPIOC->AFR[1] &= ~(1U<<11);

	// PC11
	GPIOC->AFR[1] &= ~(1U<<12);
	GPIOC->AFR[1] &= ~(1U<<13);
	GPIOC->AFR[1] &= ~(1U<<14);
	GPIOC->AFR[1] &= ~(1U<<15);

	// PC12
	GPIOC->AFR[1] &= ~(1U<<16);
	GPIOC->AFR[1] &= ~(1U<<17);
	GPIOC->AFR[1] &= ~(1U<<18);
	GPIOC->AFR[1] &= ~(1U<<19);
}

static void spi_config(SPIObject * self)
{
	SPI_TypeDef * SPIx = get_spi_module_from_number(self->spiNumber);

	// enable clock access to SPI
	if(self->spiNumber == SPI_OBJ_1)
	{
		RCC->APB2ENR |= (SPI1EN);
	}
	else if(self->spiNumber == SPI_OBJ_2)
	{
		RCC->APB1ENR |= (SPI2EN);
	}
	else if(self->spiNumber == SPI_OBJ_3)
	{
		RCC->APB1ENR |= (SPI3EN);
	}

	// set clock prescaler to fPCLK/16
//	SPIx->CR1 |= (1U<<3);
//	SPIx->CR1 |= (1U<<4);
//	SPIx->CR1 &= ~(1U<<5);

	set_baudrate_prescaler(self);

	// set CPOL to 1 and CPHA to 1
	SPIx->CR1 |= (1U<<0);
	SPIx->CR1 |= (1U<<1);

	// enable full duplex
	SPIx->CR1 &= ~(1U<<10);

	// set MSB first
	SPIx->CR1 &= ~(1U<<7);

	// set master mode
	SPIx->CR1 |= (1U<<2);

	// set 8 bit data mode
	SPIx->CR1 &= ~(1U<<11);

	// select software slave management by
	// setting SSM=1 and SSI=1
	SPIx->CR1 |= (1<<8);
	SPIx->CR1 |= (1<<9);

	// enable SPI module
	SPIx->CR1 |= (1<<6);
}

spi_result_t spi_transmit(SPIObject * self, uint8_t *data, uint32_t size)
{
	if(validate_self(self) != SPI_OK)
	{
		return SPI_ERROR_INVALID_ARGUMENT;
	}

	uint32_t i = 0;
	uint8_t temp = 0;
	temp = temp;
	SPI_TypeDef * SPIx = get_spi_module_from_number(self->spiNumber);

	while(i < size)
	{
		// wait until TXE is set (transmit buffer empty)
		while(!(SPIx->SR & (SR_TXE))){}

		// write the data to the data register
		SPIx->DR = data[i];
		i++;
	}
	// wait until TXE is set
	while(!(SPIx->SR & (SR_TXE))){}

	// wait for busy flag to reset
	while(SPIx->SR & (SR_BSY)){}

	// clear the OVR (overrun) flag
	temp = SPIx->DR;
	temp = SPIx->SR;

	return SPI_OK;
}

spi_result_t spi_receive(SPIObject * self, uint8_t *data, uint32_t size)
{
	if(validate_self(self) != SPI_OK)
	{
		return SPI_ERROR_INVALID_ARGUMENT;
	}
	SPI_TypeDef * SPIx = get_spi_module_from_number(self->spiNumber);

	while(size)
	{
		// send dummy data
		SPIx->DR = 0;

		// wait for RXNE flag to be set
		while(!(SPIx->SR & (SR_RXNE))){}

		// read the data from data register
		*data++ = (SPIx->DR);
		size--;
	}
	return SPI_OK;
}

spi_result_t cs_enable(SPIObject * self)
{
	if(validate_self(self) != SPI_OK)
	{
		return SPI_ERROR_INVALID_ARGUMENT;
	}
	// set low to enable !
	self->chipSelectPinPort->ODR &= ~(1U<<self->chipSelectPinNumber);
	return SPI_OK;
}

spi_result_t cs_disable(SPIObject * self)
{
	if(validate_self(self) != SPI_OK)
	{
		return SPI_ERROR_INVALID_ARGUMENT;
	}
	// set high to disable !
	self->chipSelectPinPort->ODR |= (1U<<self->chipSelectPinNumber);
	return SPI_OK;
}

static spi_result_t validate_self(SPIObject * self)
{
	if(self == NULL)
	{
		return SPI_ERROR_INVALID_ARGUMENT;
	}
	if(self->spiNumber != SPI_OBJ_1 && self->spiNumber != SPI_OBJ_2 && self->spiNumber != SPI_OBJ_3)
	{
		return SPI_ERROR_INVALID_ARGUMENT;
	}
	return SPI_OK;
}

static void set_baudrate_prescaler(SPIObject * self)
{
	SPI_TypeDef * SPIx = get_spi_module_from_number(self->spiNumber);

	SPIx->CR1 &= ~(0x07U<<3);

	switch(self->baudRatePrescaler)
	{
/*	case 2:
		SPIx->CR1 |= (0x00U<<3);
		break;						*/
	case 4:
		SPIx->CR1 |= (0x01U<<3);
		break;
	case 8:
		SPIx->CR1 |= (0x02U<<3);
		break;
	case 16:
		SPIx->CR1 |= (0x03U<<3);
		break;
	case 32:
		SPIx->CR1 |= (0x04U<<3);
		break;
	case 64:
		SPIx->CR1 |= (0x05U<<3);
		break;
	case 128:
		SPIx->CR1 |= (0x06U<<3);
		break;
	case 256:
		SPIx->CR1 |= (0x07U<<3);
		break;
	default:
		break;
	}
}









