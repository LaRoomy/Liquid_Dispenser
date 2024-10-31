#include "main.h"

/*
 * Test 6 - CAN
 *
 * CAN is initialized in loop-back mode and the std test id defined in can.h is used.
 * When the user button is pressed, the test cases will be executed one after each other
 * and different transmission package sizes will be tested. When successful the data received
 * interrupt is triggered and the transmission reception handling signals transmission complete,
 * then an assert is executed for each test case.
 *
 * */

uint8_t testCaseCounter = 0;

// can test data
uint8_t canTestDataSingleByte = 108;
uint8_t canTestDataSixByteLength[6] = {20,21,22,23,24,25};
uint8_t canTestDataEightByteLength[8] = {5,101,202,70,19,155,251,96};
uint8_t canTestDataTwelveByteLength[12] = {11,12,13,14,15,16,17,18,19,20,21,22};
uint8_t canTestDataSixteenByteLength[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
uint8_t canTestDataThirtyTwoByteLength[32] = {16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

// can callback functions
void on_can_data_received(CanTransmissionData * tData);
void on_can_error_occurred(uint32_t errorCode);

int main(void)
{
	CPU_Clock_Config();

	init_board_led();
	init_board_switch();
	init_test_tools(SYS_FRQ);

	// init can in loop back mode for testing purposes (no filters needed in loop back mode)
	can_init(CAN_LOOP_BACK_MODE, 0, NULL);

	CanCallback canCallback =
	{
		.dataReceived = on_can_data_received,
		.errorOccurred = on_can_error_occurred
	};
	can_set_callback(&canCallback);

	while(1)
	{
		if(get_board_switch_status_with_debounce() == PRESSED)
		{
			if(testCaseCounter > 5)
			{
				testCaseCounter = 0;
			}

			control_board_led(ON);

			CanTransmissionData tData;
			tData.stdID = CAN_STD_LOOP_BACK_TEST_ID;

			if(testCaseCounter == 0)
			{
				printf("FIRST TEST EXECUTION: Send and receive a single byte!\r\n\r\n");

				tData.data = &canTestDataSingleByte;
				tData.dataSize = 1;
			}
			else if(testCaseCounter == 1)
			{
				printf("SECOND TEST EXECUTION: Send and receive 8 byte\r\n\r\n");

				tData.data = canTestDataEightByteLength;
				tData.dataSize = 8;
			}
			else if(testCaseCounter == 2)
			{
				printf("THIRD TEST EXECUTION: Send and receive 6 byte\r\n\r\n");

				tData.data = canTestDataSixByteLength;
				tData.dataSize = 6;
			}
			else if(testCaseCounter == 3)
			{
				printf("FOUTH TEST EXECUTION: Send and receive a large buffer exceeding the 8 byte limit but remain in the 18 byte limit!\r\n\r\n");

				tData.data = canTestDataSixteenByteLength;
				tData.dataSize = 16;
			}
			else if(testCaseCounter == 4)
			{
				printf("FIFTH TEST EXECUTION: Send and receive a large buffer exceeding the 8 byte limit and exceed the 18 byte limit!\r\n\r\n");

				tData.data = canTestDataThirtyTwoByteLength;
				tData.dataSize = 32;
			}
			else if(testCaseCounter == 5)
			{
				printf("SIXTH TEST EXECUTION: Send and receive 12 byte\r\n\r\n");

				tData.data = canTestDataTwelveByteLength;
				tData.dataSize = 12;
			}
			else
			{
				// should never be reached
				testCaseCounter = 0;
			}

			can_send_data(&tData);
		}
	}
}

void on_can_data_received(CanTransmissionData * tData)
{
	if(testCaseCounter == 0)
	{
		printf("Can RX Interrupt: CAN1 single byte transmission.\r\n\r\n");

		testCaseCounter++;

		printf("Input buffer complete! Data: %u\r\n\r\n", (unsigned int)tData->data[0]);

		assert_uint_are_equal(canTestDataSingleByte, tData->data[0], "CAN Single byte transmission");
		assert_uint_are_equal(1, tData->dataSize, "Assertion of transmission data size");
	}
	else if(testCaseCounter == 1)
	{
		printf("Can RX Interrupt: CAN1 8 byte transmission.\r\n\r\n");

		testCaseCounter++;

		printf("Input buffer complete! Data: ");

		for(int i = 0; i < 8; i++)
		{
			printf("%u ", (unsigned int)tData->data[i]);
		}
		printf("\r\n\r\n");
		assert_u8_arrays_are_equal(canTestDataEightByteLength, tData->data, 8, "CAN 8 byte transmission");
		assert_uint_are_equal(8, tData->dataSize, "Assertion of transmission data size");

	}
	else if(testCaseCounter == 2)
	{
		printf("Can RX Interrupt: CAN1 6 byte transmission.\r\n\r\n");

		testCaseCounter++;

		printf("Input buffer complete! Data: ");

		for(int i = 0; i < 6; i++)
		{
			printf("%u ", (unsigned int)tData->data[i]);
		}
		printf("\r\n\r\n");
		assert_u8_arrays_are_equal(canTestDataSixByteLength, tData->data, 6, "CAN 6 byte transmission");
		assert_uint_are_equal(6, tData->dataSize, "Assertion of transmission data size");
	}
	else if(testCaseCounter == 3)
	{
		printf("Interrupt reported: CAN1 large data transmission 1 received. (16 byte)\r\n\r\n");

		testCaseCounter++;

		printf("Input buffer complete! Data: ");

		for(int i = 0; i < 16; i++)
		{
			printf("%u ", (unsigned int)tData->data[i]);
		}
		printf("\r\n\r\n");
		assert_u8_arrays_are_equal(canTestDataSixteenByteLength, tData->data, 16, "Input data array of overlength transmission");
		assert_uint_are_equal(16, tData->dataSize, "Assertion of transmission data size");
	}
	else if(testCaseCounter == 4)
	{
		printf("Interrupt reported: CAN1 large data transmission 2 received. (32 byte)\r\n\r\n");

		testCaseCounter++;

		printf("Input buffer complete! Data: ");

		for(int i = 0; i < 32; i++)
		{
			printf("%u ", (unsigned int)tData->data[i]);
		}
		printf("\r\n\r\n");
		assert_u8_arrays_are_equal(canTestDataThirtyTwoByteLength, tData->data, 32, "Input data array of overlength transmission");
		assert_uint_are_equal(32, tData->dataSize, "Assertion of transmission data size");
	}
	else if(testCaseCounter == 5)
	{
		printf("Can RX Interrupt: CAN1 12 byte transmission.\r\n\r\n");

		testCaseCounter++;

		printf("Input buffer complete! Data: ");

		for(int i = 0; i < 12; i++)
		{
			printf("%u ", (unsigned int)tData->data[i]);
		}
		printf("\r\n\r\n");
		assert_u8_arrays_are_equal(canTestDataTwelveByteLength, tData->data, 12, "CAN 12 byte transmission");
		assert_uint_are_equal(12, tData->dataSize, "Assertion of transmission data size");
	}
	control_board_led(OFF);
}

void on_can_error_occurred(uint32_t errorCode)
{
	printf("Error occurred in CAN module!\r\n");
	printf("Error Code: %u\r\n", (unsigned int)errorCode);
	// 001 = 1 -> stuff error
	// 010 = 2 -> form error
	// 011 = 3 -> acknowledgment error
	// 100 = 4 -> bit recessive error
	// 101 = 5 -> bit dominant error
	// 110 = 6 -> crc error
	// 111 = 7 -> set by software
}






