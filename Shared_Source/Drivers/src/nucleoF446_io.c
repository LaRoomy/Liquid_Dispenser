#include <stm32f4xx.h>
#include <stm32f4xx_tools.h>
#include <nucleoF446_io.h>

void init_board_led()
{
	/* configure PA5 as output pin*/
	gpio_config_port_pin(GPIOA, 5, GPIO_MODE_OUTPUT);
}

void control_board_led(int status)
{
	gpio_control_pin(GPIOA, 5, status);
}

void init_board_switch()
{
	/* configure PC13 as input pin*/
	gpio_config_port_pin(GPIOC, 13, GPIO_MODE_INPUT);
}

uint32_t get_board_switch_status()
{
	if(gpio_read_pin(GPIOC, 13) == LOW)
	{
		return PRESSED;
	}
	else
	{
		return INACTIVE;
	}
}

uint32_t get_board_switch_status_with_debounce()
{
	if(gpio_read_pin(GPIOC, 13) == LOW)
	{
		for(int i = 0; i < 1000; i++){}
		while(gpio_read_pin(GPIOC, 13) == LOW){}
		for(int i = 0; i < 3000; i++){}
		return PRESSED;
	}
	else
	{
		return INACTIVE;
	}
}



