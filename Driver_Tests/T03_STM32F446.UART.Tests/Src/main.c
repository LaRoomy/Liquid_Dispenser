#include "main.h"

/*
 * Test 3 - UART
 *
 * The uart tx line is tested by pressing the user push button. Two transmitter tests
 * are executed. The first is using the uart_write function directly. The second uses the
 * rerouted printf function.
 * The uart rx line is tested by receiving a character and consequently activating the user led of the
 * nucleo board if the character '1' is received and deactivates it if any other char is received.
 * */

void uart_reception_callback(char data, uint32_t uartNumber);

UARTObject uartObj =
{
	.apbClockFrequency = SYS_FRQ,
	.baudRate = 115200U,
	.uartNumber = 2,
	.transferDirection = UART_TRANSFER_DIR_RXTX,
	.uart_reception_callback = uart_reception_callback
};

uint32_t testCaseCounter = 0;

int main(void)
{
	CPU_Clock_Config();

	init_board_led();
	init_board_switch();

	uart_init(&uartObj);

	while(1)
	{
		if(get_board_switch_status_with_debounce() == PRESSED)
		{
			if(testCaseCounter == 0)
			{
				char str[] = "Test of uart2 write function\r\n\r\n\0";
				uart_write(&uartObj, str, strlen(str));
			}
			else if(testCaseCounter == 1)
			{
				char str[] = "Test of uart printf function\r\n\r\n\0";
				printf(str);
			}
			testCaseCounter++;
			if(testCaseCounter > 1)
			{
				testCaseCounter = 0;
			}
		}
	}
}

void uart_reception_callback(char data, uint32_t uartNumber)
{
	if(data == '1')
	{
		control_board_led(ON);
	}
	else
	{
		control_board_led(OFF);
	}
}




