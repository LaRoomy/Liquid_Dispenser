#include "main.h"

#define		FILTER_LIST_LENGTH		4

#define	CAN_DEVICE_1

#ifndef CAN_DEVICE_1
uint32_t inputIdFilterList[FILTER_LIST_LENGTH] = { 0xE9, 0xF3, 0x1C1 ,0x2FF };
uint32_t outputIdFilterList[FILTER_LIST_LENGTH] = { 0xE1, 0xF8, 0x1B2 ,0x300 };
#else
uint32_t inputIdFilterList[FILTER_LIST_LENGTH] = { 0xE1, 0xF8, 0x1B2 ,0x300 };
uint32_t outputIdFilterList[FILTER_LIST_LENGTH] = { 0xE9, 0xF3, 0x1C1 ,0x2FF };
#endif

/*
 * Test 6 - CAN Communication between two devices
 *
 * When the user button is pressed, the test cases will be executed one after each other
 * and the every message is transmitted with another std id for the reception filter of the
 * remote device. The std-ids are provided above for two devices. By enabling or disabling
 * the define 'CAN_DEVICE_1' the values are switched from input and output, so that the input filter
 * values of the first device are the target output std-id values of the second device and
 * vice versa.
 *
 * */

uint8_t testCaseCounter = 0;

// can callback functions
void on_can_data_received(CanTransmissionData * tData);
void on_can_error_occurred(uint32_t errorCode);

int main(void)
{
	CPU_Clock_Config();

	init_board_led();
	init_board_switch();
	init_test_tools(SYS_FRQ);

	// init can and set the filters for messages
	can_init(CAN_NORMAL_MODE, FILTER_LIST_LENGTH, inputIdFilterList);

	CanCallback canCallback =
	{
		.dataReceived = on_can_data_received,
		.errorOccurred = on_can_error_occurred
	};
	can_set_callback(&canCallback);

#ifdef CAN_DEVICE_1
	control_board_led(ON);
#endif

	while(1)
	{
		if(get_board_switch_status_with_debounce() == PRESSED)
		{
			if(testCaseCounter > 3)
			{
				testCaseCounter = 0;
			}

			CanTransmissionData tData;
			tData.data = NULL;

			if(testCaseCounter == 0)
			{
				printf("Send message for first filter\r\n\r\n");

				tData.data =
						string_to_u8_array("1 - Message for first filter.", &tData.dataSize);
				tData.stdID = outputIdFilterList[0];
			}
			else if(testCaseCounter == 1)
			{
				printf("Send message for second filter\r\n\r\n");

				tData.data =
						string_to_u8_array("2 - Message for second filter.", &tData.dataSize);
				tData.stdID = outputIdFilterList[1];
			}
			else if(testCaseCounter == 2)
			{
				printf("Send message for third filter\r\n\r\n");

				tData.data =
						string_to_u8_array("3 - Message for third filter.", &tData.dataSize);
				tData.stdID = outputIdFilterList[2];
			}
			else if(testCaseCounter == 3)
			{
				printf("Send message for fourth filter\r\n\r\n");

				tData.data =
						string_to_u8_array("4 - Message for fourth filter.", &tData.dataSize);
				tData.stdID = outputIdFilterList[3];
			}

			can_send_data(&tData);

			if(tData.data != NULL)
			{
				free(tData.data);
			}

			testCaseCounter++;
		}
	}
}

void on_can_data_received(CanTransmissionData * tData)
{
	printf("CAN data received:\r\n\r\n");

	char* strData = u8_array_to_string(tData->data, tData->dataSize);

	if(tData->stdID == inputIdFilterList[0])
	{
		printf("  >> Match for std id with index 0\r\n  -> Data: %s\r\n\r\n", strData);
	}
	else if(tData->stdID == inputIdFilterList[1])
	{
		printf("  >> Match for std id with index 1\r\n  -> Data: %s\r\n\r\n", strData);
	}
	else if(tData->stdID == inputIdFilterList[2])
	{
		printf("  >> Match for std id with index 2\r\n  -> Data: %s\r\n\r\n", strData);
	}
	else if(tData->stdID == inputIdFilterList[3])
	{
		printf("  >> Match for std id with index 3\r\n  -> Data: %s\r\n\r\n", strData);
	}
	else
	{
		printf("ERROR: unknown std id match! Value: %u \r\n\r\n", (unsigned int)tData->stdID);
	}
	free(strData);
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





