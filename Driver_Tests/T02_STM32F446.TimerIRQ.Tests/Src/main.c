#include "main.h"

/*
 * Test 2 - Timer interrupt test for timer 1 - 14
 *
 * The timer generates a 2Hz interrupt which toggles the user led.
 * Pushing the user button stopps the current timer and deinitializes it. The user led
 * is in a steady state for one second to indicate the transition. Then the timer number
 * is increased and the next timer is initialized. This is executed from timer 1 up to
 * timer 14. Then the blink rate is doubled and the sequence starts again starting with timer 1.
 * If an error occurs in timer initialization, the user led is switched on and the program is
 * suspended in error state.
 *
 * */

void timer_callback(void * param);
uint32_t get_next_timer_number(uint32_t timerNumber);

uint32_t errorState = 0;
uint32_t checkupParam = 1;

TimerObject timerObj =
{
	.systemFrequency = SYS_FRQ,
	.millisecondRate = 500,
	.timerNumber = 1U,
	.timer_callback = timer_callback,
	.cb_param = &checkupParam
};

int main(void)
{
	CPU_Clock_Config();

	init_board_led();
	init_board_switch();

	tim_interrupt_init(&timerObj);

	while(1)
	{
		if(get_board_switch_status_with_debounce() == PRESSED)
		{
			if(errorState == 0)
			{
				tim_deinit(&timerObj);
				control_board_led(ON);
				delay_ms(1000, SYS_FRQ);

				if(timerObj.timerNumber == 14U)
				{
					timerObj.millisecondRate /= 2U;
				}

				timerObj.timerNumber =
						get_next_timer_number(timerObj.timerNumber);

				checkupParam = timerObj.timerNumber;

				if(tim_interrupt_init(&timerObj) == TIMER_ERROR_INVALID_PARAMETER)
				{
					control_board_led(ON);
					errorState = 1;
				}
			}
		}
	}
}

void timer_callback(void * param)
{
	uint32_t vCheck = *((uint32_t*)param);
	if(vCheck == timerObj.timerNumber)
	{
		control_board_led(TOGGLE);
	}
}

uint32_t get_next_timer_number(uint32_t timerNumber)
{
	if(timerNumber == 14U)
	{
		return 1U;
	}
	else
	{
		return (timerNumber + 1U);
	}
}



