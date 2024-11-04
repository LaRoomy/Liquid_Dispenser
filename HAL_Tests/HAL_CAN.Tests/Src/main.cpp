#include "main.hpp"

constexpr uint32_t MaxFilters = 3;

uint32_t testResult;
uint32_t testCounter;
uint32_t testResultNumber;

uint32_t filterList[] = { 0x01U , 0xA2U, 0xE1U };

uint8_t testData_1[] = { 22, 23, 24 };
uint8_t testData_2[] = { 1, 233, 56, 101, 99 };
uint8_t testData_3[] = { 34, 94, 200, 255, 126, 5, 8, 21, 121, 75, 83, 222, 230, 242 };

// helper
int datacmp(uint8_t * data1, uint32_t dataSize1, uint8_t * data2, uint32_t dataSize2);
void canDataFromCounter(PCAN_DATA pCanData);
void logMessage(ISerial * pSerial, const char * msg, uint32_t testDataIndex);

class CanCallback : public ICanCallback
{
public:
	void canDataReceived(PCAN_DATA pCanData) override
	{
		testResult = 2;

		if(pCanData->stdID == filterList[0])
		{
			if(datacmp(testData_1, 3, pCanData->data, pCanData->dataSize) == 0)
			{
				testResult = 1;
				testResultNumber = 1;
			}
		}
		else if(pCanData->stdID == filterList[1])
		{
			if(datacmp(testData_2, 5, pCanData->data, pCanData->dataSize) == 0)
			{
				testResult = 1;
				testResultNumber = 2;
			}
		}
		else if(pCanData->stdID == filterList[2])
		{
			if(datacmp(testData_3, 14, pCanData->data, pCanData->dataSize) == 0)
			{
				testResult = 1;
				testResultNumber = 3;
			}
		}
	}
	void canErrorOccurred(const char * errInfo) override
	{
		for(;;);
	}
};

CanCallback canCallback;

int main(void)
{
	CAN_FILTERS canFilters = {
		.stdIDFilterList = filterList,
		.numFilters = MaxFilters
	};
	testResult = 0;
	testCounter = 0;
	testResultNumber = 0;

	CPU_Clock_Config();
	init_board_led();
	init_board_switch();

	// log
	auto serialLog = CreateSerialObject();

	// can
	auto canInstance = GetCanControllerInstance();
	canInstance->init(&canFilters);
	canInstance->registerCallback(&canCallback);

	while(1)
	{
		if(get_board_switch_status_with_debounce() == PRESSED)
		{
			logMessage(serialLog, "Writing CAN data!", testCounter + 1);

			CAN_DATA canData;
			canDataFromCounter(&canData);
			canInstance->writeData(&canData);
		}

		if(testResult != 0)
		{
			if(testResult == 1)
			{
				logMessage(serialLog, "--->> Test succeeded!", testResultNumber);
			}
			else
			{
				logMessage(serialLog, "--->> Test failed!", testResultNumber);
			}
			testResult = 0;
		}
	}
}

int datacmp(uint8_t * data1, uint32_t dataSize1, uint8_t * data2, uint32_t dataSize2)
{
	if(dataSize1 == dataSize2)
	{
		for(uint32_t i = 0; i < dataSize1; i++)
		{
			if(data1[i] != data2[i])
			{
				return 1;
			}
		}
		return 0;
	}
	return 1;
}

void canDataFromCounter(PCAN_DATA pCanData)
{
	pCanData->stdID = filterList[testCounter];

	switch(testCounter)
	{
	case 0:
		pCanData->data = testData_1;
		pCanData->dataSize = 3;
		break;
	case 1:
		pCanData->data = testData_2;
		pCanData->dataSize = 5;
		break;
	case 2:
		pCanData->data = testData_3;
		pCanData->dataSize = 14;
		break;
	default:
		break;
	}

	testCounter++;
	if(testCounter > 2)
	{
		testCounter = 0;
	}
}

void logMessage(ISerial * pSerial, const char * msg, uint32_t testDataIndex)
{
	if(pSerial != nullptr)
	{
		char outMsg[255] = { '\0' };
		sprintf(outMsg, "%s\r\n >> Subject: TestData %u\r\n\r\n", msg, (unsigned int)testDataIndex);
		pSerial->write(outMsg);
	}
}




