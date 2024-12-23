#include "main.h"

/*
 * Test 12 - External Interrupts
 *
 * By pressing the board switch a single test case is executed and the result is printed
 * through the serial monitor. Each available port on the development bord ist tested.
 * The trigger on the pin is generated by another pin, so the interrupt test pins must be connected
 * to the trigger pins:
 *
 * | Int Pin | Trigger Pin |
 * |-----------------------|
 * | PA6     | PB1		   |
 * | PB8     | PC4         |
 * | PC7     | PA12        |
 *
 * */

void external_interrupt_callback_PA6();
void external_interrupt_callback_PB8();
void external_interrupt_callback_PC7();

ExternalInterruptObject extIntTestObj_1 =
{
	.extIntPin = 6,
	.extIntPort = GPIOA,
	.triggerType = EXT_INT_TRIGGER_TYPE_FALLING_EDGE,
	.ext_int_callback = external_interrupt_callback_PA6
};

ExternalInterruptObject extIntTestObj_2 =
{
	.extIntPin = 8,
	.extIntPort = GPIOB,
	.triggerType = EXT_INT_TRIGGER_TYPE_FALLING_EDGE,
	.ext_int_callback = external_interrupt_callback_PB8
};

ExternalInterruptObject extIntTestObj_3 =
{
	.extIntPin = 7,
	.extIntPort = GPIOC,
	.triggerType = EXT_INT_TRIGGER_TYPE_RISING_EDGE,
	.ext_int_callback = external_interrupt_callback_PC7
};

uint32_t testCaseCounter = 0;
volatile uint32_t testActive = 0;
volatile uint32_t timeoutCounter = 0;

int main(void)
{
	CPU_Clock_Config();

	init_board_led();
	init_board_switch();
	init_test_tools(SYS_FRQ);

	gpio_config_port_pin(GPIOB, 1, GPIO_MODE_OUTPUT);  // trigger pin for test object 1 (connected to PA6)
	gpio_control_pin(GPIOB, 1, HIGH);
	gpio_config_port_pin(GPIOC, 4, GPIO_MODE_OUTPUT);  // trigger pin for test object 2 (connected to PB8)
	gpio_control_pin(GPIOC, 4, HIGH);
	gpio_config_port_pin(GPIOA, 12, GPIO_MODE_OUTPUT); // trigger pin for test object 3 (connected to PC7)
	gpio_control_pin(GPIOA, 12, LOW);

	external_interrupt_init(&extIntTestObj_1);
	external_interrupt_init(&extIntTestObj_2);
	external_interrupt_init(&extIntTestObj_3);

	while(1)
	{
		if(get_board_switch_status_with_debounce() == PRESSED)
		{
			testActive = 1;
			timeoutCounter = 0;

			if(testCaseCounter == 0)
			{
				printf("TEST 1 - generating falling edge trigger on PA6 >>\r\n\r\n");
				gpio_control_pin(GPIOB, 1, LOW);

				testCaseCounter = 1;
			}
			else if(testCaseCounter == 1)
			{
				printf("TEST 2 - generating falling edge trigger on PB8 >>\r\n\r\n");
				gpio_control_pin(GPIOC, 4, LOW);

				testCaseCounter = 2;
			}
			else if(testCaseCounter == 2)
			{
				printf("TEST 3 - generating rising edge trigger on PC7 >>\r\n\r\n");
				gpio_control_pin(GPIOA, 12, HIGH);

				testCaseCounter = 0;
			}
			else
			{
				testCaseCounter = 0;
			}
		}
		if(testActive != 0)
		{
			timeoutCounter++;

			if(timeoutCounter > 50)
			{
				printf("Timeout for Test %u - TEST FAILED!\r\n\r\n", (unsigned int)testCaseCounter);
				testActive = 0;
				timeoutCounter = 0;
			}
		}
	}
}

void external_interrupt_callback_PA6()
{
	assert_uint_are_equal(0, testCaseCounter, "IRQ on PA6 - Check for correct interrupt invokation");
	gpio_control_pin(GPIOB, 1, HIGH);
	testActive = 0;
}

void external_interrupt_callback_PB8()
{
	assert_uint_are_equal(1, testCaseCounter, "IRQ on PB8 - Check for correct interrupt invokation");
	gpio_control_pin(GPIOC, 4, HIGH);
	testActive = 0;
}

void external_interrupt_callback_PC7()
{
	assert_uint_are_equal(2, testCaseCounter, "IRQ on PC7 - Check for correct interrupt invokation");
	gpio_control_pin(GPIOA, 12, LOW);
	testActive = 0;
}






