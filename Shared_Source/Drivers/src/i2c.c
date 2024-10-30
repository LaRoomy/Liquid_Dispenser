#include <i2c.h>
#include <stm32f4xx_tools.h>
#include <common.h>

#define		I2C1EN		(1U<<21)
#define		I2C2EN		(1U<<22)
#define		I2C3EN		(1U<<23)

#define		I2C_100KHZ				(0xb4)
#define		SD_MODE_MAX_RISE_TIME	(0x25)
#define		CR1_PE					(1U<<0)

#define		SR2_BUSY				(1U<<1)
#define		CR1_START				(1U<<8)
#define		SR1_SB					(1U<<0)
#define		SR1_ADDR				(1U<<1)
#define		SR1_TXE					(1U<<7)
#define		CR1_ACK					(1U<<10)
#define		CR1_STOP				(1U<<9)
#define		SR1_RXNE				(1U<<6)
#define		SR1_BTF					(1U<<2)

static void i2c_config_port_for_i2c_number(uint32_t i2cNumber);
static void reset_port_for_i2c_number(uint32_t i2cNumber);
static I2C_TypeDef * get_i2c_mod_from_number(uint32_t i2cNumber);
static void enable_clock_access_from_i2c_number(uint32_t i2cNumber);
static void disable_clock_access_from_i2c_number(uint32_t i2cNumber);

static i2c_result_t validate_i2c_object(I2CObject * self)
{
	if(self == NULL)
	{
		return I2C_ERROR_INVALID_ARGUMENT;
	}
	if(self->apbClockFrequency == 0 || self->memoryAddressFormat > I2C_MEM_ADDR_FORMAT_16BIT)
	{
		return I2C_ERROR_INVALID_ARGUMENT;
	}
	if(self->i2cNumber < 1 || self->i2cNumber > 3)
	{
		return I2C_ERROR_INVALID_ARGUMENT;
	}
	return I2C_OK;
}

i2c_result_t I2C_init(I2CObject * self)
{
	if(validate_i2c_object(self) != I2C_OK)
	{
		return I2C_ERROR_INVALID_ARGUMENT;
	}

	uint32_t freq = 8;
	if(self->apbClockFrequency >= 2000000)
	{
		freq = self->apbClockFrequency / 1000000;
	}

	i2c_config_port_for_i2c_number(self->i2cNumber);

	I2C_TypeDef * I2Cx =
			get_i2c_mod_from_number(self->i2cNumber);

	// disable I2C
	I2Cx->CR1 &= ~(CR1_PE);

	enable_clock_access_from_i2c_number(self->i2cNumber);

	// enter reset mode
	I2Cx->CR1 |= (1U<<15);

	// leave reset mode
	I2Cx->CR1 &= ~(1U<<15);

	// set peripheral clock frequency
	I2Cx->CR2 |= freq;

	// set I2C to standard mode (100kHz)
	I2Cx->CCR = (I2C_100KHZ);				// TODO: adapt to freq

	// set rise time
	I2Cx->TRISE = (SD_MODE_MAX_RISE_TIME);	// TODO: adapt to freq

	// enable I2C
	I2Cx->CR1 |= CR1_PE;

	return I2C_OK;
}

i2c_result_t I2C_deinit(I2CObject * self)
{
	if(validate_i2c_object(self) != I2C_OK)
	{
		return I2C_ERROR_INVALID_ARGUMENT;
	}

	I2C_TypeDef * I2Cx =
			get_i2c_mod_from_number(self->i2cNumber);

	// disable I2C
	I2Cx->CR1 &= ~(CR1_PE);

	disable_clock_access_from_i2c_number(self->i2cNumber);
	reset_port_for_i2c_number(self->i2cNumber);

	return I2C_OK;
}

i2c_result_t I2C_read(I2CObject * self, I2CData * transmissionData)
{
	if(self == NULL || transmissionData == NULL)
	{
		return I2C_ERROR_INVALID_ARGUMENT;
	}

	volatile int tmp = 0;
	tmp = tmp;
	uint32_t nBytes = transmissionData->lengthOfData;

	I2C_TypeDef * I2Cx =
			get_i2c_mod_from_number(self->i2cNumber);

	if(I2Cx == NULL)
	{
		return I2C_ERROR_INVALID_ARGUMENT;
	}

	// wait until line is free (check busy flag)
	while(I2Cx->SR2 & (SR2_BUSY)){}

	// generate start condition
	I2Cx->CR1 |= CR1_START;

	// wait for start flag set
	while(!(I2Cx->SR1 & (SR1_SB))){}

	// transmit slave address + write
	I2Cx->DR = transmissionData->slaveAddress << 1;

	// wait until addr flag is set
	while(!(I2Cx->SR1 & (SR1_ADDR))){}

	// clear addr flag (by reading the status register 2)
	tmp = I2Cx->SR2;

	// wait until transmitter is empty
	while(!(I2Cx->SR1 & (SR1_TXE))){}

	// ---------------

	if(self->memoryAddressFormat == I2C_MEM_ADDR_FORMAT_16BIT)
	{
		// send high-byte memory address
		I2Cx->DR = (uint8_t)(transmissionData->memoryAddress>>8);

		// wait until transmitter is empty
		while(!(I2Cx->SR1 & (SR1_TXE))){}
	}

	// send low-byte memory address
	I2Cx->DR = (uint8_t)(transmissionData->memoryAddress & 0x00ff);

	// wait until transmitter is empty
	while(!(I2Cx->SR1 & (SR1_TXE))){}

	// ---------------

	// generate re-start condition
	I2Cx->CR1 |= CR1_START;

	// wait for start flag set
	while(!(I2Cx->SR1 & (SR1_SB))){}

	// transmit slave address + read
	I2Cx->DR = transmissionData->slaveAddress << 1 | 1;

	// wait until addr flag is set
	while(!(I2Cx->SR1 & (SR1_ADDR))){}

	// clear addr flag (by reading the status register 2)
	tmp = I2Cx->SR2;

	// enable acknowledge
	I2Cx->CR1 |= CR1_ACK;

	while(nBytes > 0U)
	{
		// if one byte
		if(nBytes == 1U)
		{
			// disable acknowledge
			I2Cx->CR1 &= ~CR1_ACK;

			// generate stop condition after data received
			I2Cx->CR1 |= CR1_STOP;

			// wait until RXNE flag is set
			while(!(I2Cx->SR1 & (SR1_RXNE))){}

			// read data from DR
			*transmissionData->data++ = I2Cx->DR;

			break;
		}
		else
		{
			// wait until RXNE flag is set
			while(!(I2Cx->SR1 & (SR1_RXNE))){}

			// read data from DR
			*transmissionData->data++ = I2Cx->DR;

			nBytes--;
		}
	}
	return I2C_OK;
}

i2c_result_t I2C_write(I2CObject * self, I2CData * transmissionData)
{
	if(self == NULL || transmissionData == NULL)
	{
		return I2C_ERROR_INVALID_ARGUMENT;
	}

	volatile int tmp = 0;
	tmp = tmp;

	I2C_TypeDef * I2Cx =
			get_i2c_mod_from_number(self->i2cNumber);

	if(I2Cx == NULL)
	{
		return I2C_ERROR_INVALID_ARGUMENT;
	}

	// wait until line is free (check busy flag)
	while(I2Cx->SR2 & (SR2_BUSY)){}

	// generate start condition
	I2Cx->CR1 |= CR1_START;

	// wait for start flag set
	while(!(I2Cx->SR1 & (SR1_SB))){}

	// transmit slave address + write
	I2Cx->DR = transmissionData->slaveAddress << 1;

	// wait until addr flag is set
	while(!(I2Cx->SR1 & (SR1_ADDR))){}

	// clear addr flag (by reading the status register 2)
	tmp = I2Cx->SR2;

	// wait until data register is empty
	while(!(I2Cx->SR1 & (SR1_TXE))){}

	// ------------------

	if(self->memoryAddressFormat == I2C_MEM_ADDR_FORMAT_16BIT)
	{
		// send high-byte memory address
		I2Cx->DR = (uint8_t)(transmissionData->memoryAddress>>8);

		// wait until data register is empty
		while(!(I2Cx->SR1 & (SR1_TXE))){}
	}

	// send low-byte memory address
	I2Cx->DR = (uint8_t)(transmissionData->memoryAddress & 0x00ff);

	// ---------------------

	for(int i = 0; i < transmissionData->lengthOfData; i++)
	{
		// wait until data register is empty
		while(!(I2Cx->SR1 & (SR1_TXE))){}

		// transmit data at memory address
		I2Cx->DR = *transmissionData->data++;
	}

	// wait until transfer finished
	while(!(I2Cx->SR1 & (SR1_BTF))){}

	// generate stop condition
	I2Cx->CR1 |= CR1_STOP;

	return I2C_OK;
}

static void i2c_config_port_for_i2c_number(uint32_t i2cNumber)
{
	if(i2cNumber == 1)
	{
		gpio_config_port_pin(GPIOB, 8, GPIO_MODE_ALTERNATE);
		gpio_config_port_pin(GPIOB, 9, GPIO_MODE_ALTERNATE);
		gpio_set_alternate_function_to_pin(GPIOB, 8, 4);
		gpio_set_alternate_function_to_pin(GPIOB, 9, 4);
		gpio_set_output_type(GPIOB, 8, GPIO_OUTPUT_TYPE_OPEN_DRAIN);
		gpio_set_output_type(GPIOB, 9, GPIO_OUTPUT_TYPE_OPEN_DRAIN);
	}
	else if(i2cNumber == 2)
	{
		gpio_config_port_pin(GPIOB, 10, GPIO_MODE_ALTERNATE);
		gpio_config_port_pin(GPIOB, 11, GPIO_MODE_ALTERNATE);
		gpio_set_alternate_function_to_pin(GPIOB, 10, 4);
		gpio_set_alternate_function_to_pin(GPIOB, 11, 4);
		gpio_set_output_type(GPIOB, 10, GPIO_OUTPUT_TYPE_OPEN_DRAIN);
		gpio_set_output_type(GPIOB, 11, GPIO_OUTPUT_TYPE_OPEN_DRAIN);
	}
	else if(i2cNumber == 3)
	{
		gpio_config_port_pin(GPIOA, 8, GPIO_MODE_ALTERNATE);
		gpio_config_port_pin(GPIOC, 9, GPIO_MODE_ALTERNATE);
		gpio_set_alternate_function_to_pin(GPIOA, 8, 4);
		gpio_set_alternate_function_to_pin(GPIOC, 9, 4);
		gpio_set_output_type(GPIOA, 8, GPIO_OUTPUT_TYPE_OPEN_DRAIN);
		gpio_set_output_type(GPIOC, 9, GPIO_OUTPUT_TYPE_OPEN_DRAIN);
	}
}

static void reset_port_for_i2c_number(uint32_t i2cNumber)
{
	if(i2cNumber == 1)
	{
		gpio_config_port_pin(GPIOB, 8, GPIO_MODE_RESET_STATE);
		gpio_config_port_pin(GPIOB, 9, GPIO_MODE_RESET_STATE);
		gpio_set_alternate_function_to_pin(GPIOB, 8, 0);
		gpio_set_alternate_function_to_pin(GPIOB, 9, 0);
		gpio_set_output_type(GPIOB, 8, GPIO_OUTPUT_TYPE_PUSH_PULL);
		gpio_set_output_type(GPIOB, 9, GPIO_OUTPUT_TYPE_PUSH_PULL);

	}
	else if(i2cNumber == 2)
	{
		gpio_config_port_pin(GPIOB, 10, GPIO_MODE_RESET_STATE);
		gpio_config_port_pin(GPIOB, 11, GPIO_MODE_RESET_STATE);
		gpio_set_alternate_function_to_pin(GPIOB, 10, 0);
		gpio_set_alternate_function_to_pin(GPIOB, 11, 0);
		gpio_set_output_type(GPIOB, 10, GPIO_OUTPUT_TYPE_PUSH_PULL);
		gpio_set_output_type(GPIOB, 11, GPIO_OUTPUT_TYPE_PUSH_PULL);
	}
	else if(i2cNumber == 3)
	{
		gpio_config_port_pin(GPIOA, 8, GPIO_MODE_RESET_STATE);
		gpio_config_port_pin(GPIOC, 9, GPIO_MODE_RESET_STATE);
		gpio_set_alternate_function_to_pin(GPIOA, 8, 0);
		gpio_set_alternate_function_to_pin(GPIOC, 9, 0);
		gpio_set_output_type(GPIOA, 8, GPIO_OUTPUT_TYPE_PUSH_PULL);
		gpio_set_output_type(GPIOC, 9, GPIO_OUTPUT_TYPE_PUSH_PULL);
	}
}

static I2C_TypeDef * get_i2c_mod_from_number(uint32_t i2cNumber)
{
	switch(i2cNumber)
	{
	case 1:
		return I2C1;
	case 2:
		return I2C2;
	case 3:
		return I2C3;
	default:
		return NULL;
	}
}

static void enable_clock_access_from_i2c_number(uint32_t i2cNumber)
{
	if(i2cNumber == 1)
	{
		RCC->APB1ENR |= (I2C1EN);
	}
	else if(i2cNumber == 2)
	{
		RCC->APB1ENR |= (I2C2EN);
	}
	else if(i2cNumber == 2)
	{
		RCC->APB1ENR |= (I2C3EN);
	}
}

static void disable_clock_access_from_i2c_number(uint32_t i2cNumber)
{
	if(i2cNumber == 1)
	{
		RCC->APB1ENR &= ~(I2C1EN);
	}
	else if(i2cNumber == 2)
	{
		RCC->APB1ENR &= ~(I2C2EN);
	}
	else if(i2cNumber == 2)
	{
		RCC->APB1ENR &= ~(I2C3EN);
	}
}




