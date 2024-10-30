#include "test_tools.h"
#include <common.h>
#include <uart.h>

UARTObject ttuartObj =
{
	.baudRate = 115200,
	.transferDirection = UART_TRANSFER_DIR_TX,
	.uartNumber = 2,
	.uart_reception_callback = NULL
};

void init_test_tools(uint32_t sys_clk)
{
	ttuartObj.apbClockFrequency = sys_clk;
	uart_init(&ttuartObj);
}

void assert_true(uint32_t test_val, char* opt_info)
{
	printf("** Assert True ***********************\r\n");
	if(opt_info != NULL)
	{
		printf("Subject: ");
		printf(opt_info);
		printf("\r\n");
	}
	printf("Result: ");
	if(test_val)
	{
		printf("SUCCESS");
	}
	else
	{
		printf("FAILED");
	}
	printf("\r\n**************************************\r\n\r\n");
}

void assert_uint_are_equal(uint32_t expected, uint32_t actual, char* opt_info)
{
	printf("** Assert Equal **********************\r\n");
	if(opt_info != NULL)
	{
		printf("Subject: ");
		printf(opt_info);
		printf("\r\n");
	}
	printf("Result: ");
	if(expected == actual)
	{
		printf("SUCCESS");
	}
	else
	{
		printf("FAILED");
		printf("\r\nExpected: %u\r\nActual:   %u", (unsigned int)expected, (unsigned int)actual);
	}
	printf("\r\n**************************************\r\n\r\n");
}

void assert_int_are_equal(int expected, int actual, char* opt_info)
{
	printf("** Assert Equal **********************\r\n");
	if(opt_info != NULL)
	{
		printf("Subject: ");
		printf(opt_info);
		printf("\r\n");
	}
	printf("Result: ");
	if(expected == actual)
	{
		printf("SUCCESS");
	}
	else
	{
		printf("FAILED");
		printf("\r\nExpected: %i\r\nActual:   %i", expected, actual);
	}
	printf("\r\n**************************************\r\n\r\n");
}

void assert_u8_arrays_are_equal(uint8_t* expected, uint8_t* actual, uint32_t size, char* opt_info)
{
	printf("** Assert Equal: uint8_t arrays **********************\r\n");
	if(opt_info != NULL)
	{
		printf("Subject: ");
		printf(opt_info);
		printf("\r\n");
	}
	printf("Result: ");
	int fail_index = -1;

	for(uint8_t i = 0; i < size; i++)
	{
		if(expected[i] != actual[i])
		{
			fail_index = (int)i;
			break;
		}
	}
	if(fail_index < 0)
	{
		printf("SUCCESS");
	}
	else
	{
		printf("FAILED at index: %i", fail_index);
		printf("\r\nExpected: %u\r\nActual:   %u", (unsigned int)expected[fail_index], (unsigned int)actual[fail_index]);
	}
	printf("\r\n****************************************************\r\n\r\n");
}










