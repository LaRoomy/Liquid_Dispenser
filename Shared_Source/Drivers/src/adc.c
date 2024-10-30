#include <stm32f4xx.h>
#include <stm32f4xx_tools.h>
#include <common.h>
#include "adc.h"

#define		ADC_INVALID_PIN_DEFINITION		4358

#define	ADC_SEQ_LEN	(0x00)
#define	CR2_ADON	(1U<<0)
#define	CR2_SWSTART	(1U<<30)
#define	SR_EOC		(1U<<1)

ADC_TypeDef * get_adc_from_number(uint32_t adcNumber);
uint32_t get_adc_channel_from_definition(ADCObject * self);

adc_result_t validate_adc_object(ADCObject * self)
{
	if(self == NULL)
	{
		return ADC_ERROR_INVALID_ARGUMENT;
	}
	if(self->adcNumber < ADC_MODULE_1 || self->adcNumber > 3)
	{
		return ADC_ERROR_INVALID_ARGUMENT;
	}
	return ADC_OK;
}

adc_result_t adc_init(ADCObject * self)
{
	if(validate_adc_object(self) != ADC_OK)
	{
		return ADC_ERROR_INVALID_ARGUMENT;
	}

	uint32_t adc_channel = get_adc_channel_from_definition(self);
	if(adc_channel == ADC_INVALID_PIN_DEFINITION)
	{
		return ADC_ERROR_INVALID_PIN_DEFINITION;
	}

	rcc_enable_clock_access_for_port(self->adcPort);
	gpio_config_port_pin(self->adcPort, self->adcPin, GPIO_MODE_ANALOG);

	// enable clock access to adc
	RCC->APB2ENR |= (1U<<(self->adcNumber+7U));

	ADC_TypeDef * ADCx = get_adc_from_number(self->adcNumber);

	// conversion sequence start
	ADCx->SQR3 = adc_channel;

	// conversion sequence length = 1 conversion
	ADCx->SQR1 = (ADC_SEQ_LEN);

	// enable adc module
	ADCx->CR2 |= (CR2_ADON);

	return ADC_OK;
}

adc_result_t adc_deinit(ADCObject * self)
{
	if(validate_adc_object(self) != ADC_OK)
	{
		return ADC_ERROR_INVALID_ARGUMENT;
	}

	ADC_TypeDef * ADCx = get_adc_from_number(self->adcNumber);

	// disable adc module
	ADCx->CR2 &= ~(CR2_ADON);
	ADCx->SQR3 = 0;

	// disable clock access to adc
	RCC->APB2ENR &= ~(1U<<(self->adcNumber+7U));

	// reset port pin
	gpio_config_port_pin(self->adcPort, self->adcPin, GPIO_MODE_RESET_STATE);

	return ADC_OK;
}

adc_result_t adc_read_single(ADCObject * self, uint32_t * adcValue)
{
	if(validate_adc_object(self) != ADC_OK || adcValue == NULL)
	{
		return ADC_ERROR_INVALID_ARGUMENT;
	}

	ADC_TypeDef * ADCx = get_adc_from_number(self->adcNumber);

	// start adc conversion
	ADCx->CR2 |= CR2_SWSTART;

	// wait for conversion to complete
	while(!(ADCx->SR & SR_EOC)){}

	// read converted result
	*adcValue = ADCx->DR;

	return ADC_OK;
}

adc_result_t adc_read_average(ADCObject * self, uint32_t conversionCount, uint32_t * adcAverageValue)
{
	if(validate_adc_object(self) != ADC_OK || conversionCount == 0 || adcAverageValue == NULL)
	{
		return ADC_ERROR_INVALID_ARGUMENT;
	}

	uint32_t averageValue = 0;
	uint32_t currentConversionResult = 0;

	for(uint32_t i = 0; i < conversionCount; i++)
	{
		adc_read_single(self, &currentConversionResult);
		averageValue += currentConversionResult;
	}

	*adcAverageValue = (averageValue / conversionCount);

	return ADC_OK;
}

ADC_TypeDef * get_adc_from_number(uint32_t adcNumber)
{
	switch(adcNumber)
	{
	case 1:
		return ADC1;
	case 2:
		return ADC2;
	case 3:
		return ADC3;
	default:
		return NULL;
	}
}

uint32_t get_adc_channel_from_definition(ADCObject * self)
{
	/*
	 * PC0 - ADC123_IN10
	 * PC1 - ADC123_IN11
	 * PC2 - ADC123_IN12
	 * PC3 - ADC123_IN13
	 * PC4 - ADC12_IN14
	 * PC5 - ADC12_IN15
	 *
	 * PB0 - ADC12_IN8
	 * PB1 - ADC12_IN9
	 *
	 * PA0 - ADC123_IN0 *
	 * PA1 - ADC123_IN1 *
	 * PA2 - ADC123_IN2 *
	 * PA3 - ADC123_IN3 *
	 * PA4 - ADC12_IN4  *
	 * PA5 - ADC12_IN5  *
	 * PA6 - ADC12_IN6  *
	 * PA7 - ADC12_IN7  *
	 *
	 * */
	if(self->adcPort == GPIOA)
	{
		if(self->adcPin < 8U)
		{
			if(self->adcPin < 4U)// PA0 - PA3 -> pin number is the same like the adc channel number
			{
				return self->adcPin;
			}
			else// PA4 - PA7 -> pin number is the same like the adc channel number BUT only for ADC1 + ADC2
			{
				if(self->adcNumber != ADC_MODULE_3)
				{
					return self->adcPin;
				}
			}
		}
	}
	else if(self->adcPort == GPIOB)
	{
		if(self->adcPin < 2U && self->adcNumber != ADC_MODULE_3)// PB0 - PB1 -> pin number is the same like the adc channel number BUT only for ADC1 + ADC2
		{
			return self->adcPin;
		}
	}
	else if(self->adcPort == GPIOC)
	{
		if(self->adcPin < 6U)
		{
			if(self->adcPin < 4U)// PC0 - PC3 -> pin number is the same like the adc channel number
			{
				return self->adcPin + 10U;
			}
			else
			{
				if(self->adcNumber != ADC_MODULE_3)// PC0 - PC3 -> pin number is the same like the adc channel number BUT only for ADC1 + ADC2
				{
					return self->adcPin + 10U;
				}
			}
		}
	}
	return ADC_INVALID_PIN_DEFINITION;
}


