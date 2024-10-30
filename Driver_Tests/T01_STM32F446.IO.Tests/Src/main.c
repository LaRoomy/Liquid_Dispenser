#include "main.h"

uint32_t doBlink = 0;
uint32_t delayCounter = 0;

/*
 * Test 1 - IO
 * The user push button of the STM32F446 nucleo board is used to activate the
 * user led toggle state at 1Hz blink rate
 * */

int main(void)
{
	CPU_Clock_Config();

	init_board_led();
	init_board_switch();

	while(1)
	{
		if(get_board_switch_status_with_debounce() == PRESSED)
		{
			doBlink = (doBlink != 0) ? 0 : 1;

			if(doBlink != 0)
			{
				control_board_led(ON);
			}
		}
		if(doBlink != 0)
		{
			if(delayCounter < 1000)
			{
				delayCounter += 10;
				delay_ms(10,  SYS_FRQ);
			}
			else
			{
				control_board_led(TOGGLE);
				delayCounter = 0;
			}
		}
		else
		{
			delayCounter = 0;
			control_board_led(OFF);
		}
	}
}




