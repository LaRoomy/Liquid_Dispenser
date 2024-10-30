#include "stm32f4xx_tools.h"

int rcc_enable_clock_access_for_port(GPIO_TypeDef * port)
{
	if(port == GPIOA){
		RCC->AHB1ENR |= (1U<<0);
	}
	else if(port == GPIOB){
		RCC->AHB1ENR |= (1U<<1);
	}
	else if(port == GPIOC){
		RCC->AHB1ENR |= (1U<<2);
	}
	else if(port == GPIOD){
		RCC->AHB1ENR |= (1U<<3);
	}
	else if(port == GPIOE){
		RCC->AHB1ENR |= (1U<<4);
	}
	else if(port == GPIOF){
		RCC->AHB1ENR |= (1U<<5);
	}
	else if(port == GPIOG){
		RCC->AHB1ENR |= (1U<<6);
	}
	else if(port == GPIOH){
		RCC->AHB1ENR |= (1U<<7);
	}
	else {
		return GPIO_CONFIG_ERROR_INVALID_ARGUMENT;
	}
	return GPIO_CONFIG_OK;
}

int gpio_config_port_pin(GPIO_TypeDef * port, uint32_t pin, uint32_t mode)
{
	if(is_valid_port(port) == FALSE || is_valid_pin(pin) == FALSE || (mode > GPIO_MODE_ANALOG))
	{
		return GPIO_CONFIG_ERROR_INVALID_ARGUMENT;
	}
	else
	{
		rcc_enable_clock_access_for_port(port);

		uint32_t firstModerBit = pin*2U;

		if(mode == GPIO_MODE_INPUT)
		{
			port->MODER &= ~(1U<<firstModerBit);
			port->MODER &= ~(1U<<(firstModerBit+1U));
		}
		else if(mode == GPIO_MODE_OUTPUT)
		{
			port->MODER |= (1U<<firstModerBit);
			port->MODER &= ~(1U<<(firstModerBit+1U));
		}
		else if(mode == GPIO_MODE_ALTERNATE)
		{
			port->MODER &= ~(1U<<firstModerBit);
			port->MODER |= (1U<<(firstModerBit+1U));
		}
		else if(mode == GPIO_MODE_ANALOG)
		{
			port->MODER |= (1U<<firstModerBit);
			port->MODER |= (1U<<(firstModerBit+1U));
		}
		return GPIO_CONFIG_OK;
	}
}

int gpio_set_alternate_function_to_pin(GPIO_TypeDef * port, uint32_t pin, uint32_t afNumber)
{
	if(is_valid_port(port) == FALSE || is_valid_pin(pin) == FALSE || (afNumber > 15))
	{
		return GPIO_CONFIG_ERROR_INVALID_ARGUMENT;
	}
	else
	{
		if(pin < 8) // low register
		{
			uint32_t firstAFBit = (pin*4U);

			// erase bits
			port->AFR[0] &= ~(1U<<firstAFBit);
			port->AFR[0] &= ~(1U<<(firstAFBit+1U));
			port->AFR[0] &= ~(1U<<(firstAFBit+2U));
			port->AFR[0] &= ~(1U<<(firstAFBit+3U));

/*			uint32_t mask = 0xFFFFFFFF; //(alternate way to do this)
			mask &= ~(0xF<<(pin*4U));
			port->AFR[0] &= mask;	*/

			// set alternate function
			port->AFR[0] |= (afNumber<<firstAFBit);
		}
		else // high register
		{
			uint32_t firstAFBit = ((pin-8U)*4U);

			// erase bits
			port->AFR[1] &= ~(1U<<firstAFBit);
			port->AFR[1] &= ~(1U<<(firstAFBit+1U));
			port->AFR[1] &= ~(1U<<(firstAFBit+2U));
			port->AFR[1] &= ~(1U<<(firstAFBit+3U));

			// set alternate function
			port->AFR[1] |= (afNumber<<firstAFBit);
		}
		return GPIO_CONFIG_OK;
	}
}

int gpio_set_output_type(GPIO_TypeDef * port, uint32_t pin, uint32_t type)
{
	if(is_valid_port(port) == FALSE || is_valid_pin(pin) == FALSE || (type > GPIO_OUTPUT_TYPE_OPEN_DRAIN))
	{
		return GPIO_CONFIG_ERROR_INVALID_ARGUMENT;
	}
	else
	{
		if(type == GPIO_OUTPUT_TYPE_OPEN_DRAIN)
		{
			port->OTYPER |= (1U<<pin);
		}
		else
		{
			port->OTYPER &= ~(1U<<pin);
		}
		return GPIO_CONFIG_OK;
	}
}

int gpio_control_pin(GPIO_TypeDef * port, uint32_t pin, uint32_t state)
{
	if(port == NULL)
	{
		return GPIO_CONFIG_ERROR_INVALID_ARGUMENT;
	}
	else
	{
		if(state == LOW)
		{
			port->ODR &= ~(1U<<pin);
		}
		else if(state == HIGH)
		{
			port->ODR |= (1U<<pin);
		}
		else if(state == TOGGLE)
		{
			port->ODR ^= (1U<<pin);
		}
		else
		{
			return GPIO_CONFIG_ERROR_INVALID_ARGUMENT;
		}
		return GPIO_CONFIG_OK;
	}
}

uint32_t gpio_read_pin(GPIO_TypeDef * port, uint32_t pin)
{
	return ((port->IDR & (1U<<pin)) != 0 ? HIGH : LOW);
}

BOOL is_valid_port(GPIO_TypeDef * port)
{
	if(port != GPIOA && port != GPIOB && port != GPIOC && port != GPIOD &&
			port != GPIOE && port != GPIOF && port != GPIOG && port != GPIOH)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

BOOL is_valid_pin(uint32_t pinNumber)
{
	return (pinNumber > 15) ? FALSE : TRUE;
}

BOOL is_valid_port_pin(GPIO_TypeDef * port, uint32_t pin)
{
	return (is_valid_port(port) == TRUE && is_valid_pin(pin) == TRUE) ? TRUE : FALSE;
}







