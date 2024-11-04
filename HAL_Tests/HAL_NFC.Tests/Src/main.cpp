#include "main.hpp"

uint32_t responseIndicator = 0;

class CNfcCallback : public INfcCallback
{
public:
	void nfcDataReceived(const char * data)
	{
		if(strcmp(data, "hello world") == 0)
		{
			responseIndicator = 1;
		}
		else
		{
			responseIndicator = 2;
		}
	}
};

/*
 * NOTE:
 * To test the nfc connection my android app could be used: https://github.com/LaRoomy/Android_NFC_Control
 * */

CNfcCallback callback;

int main(void)
{
	CPU_Clock_Config();
	init_board_led();
	init_board_switch();

	// log
	auto serialLog = CreateSerialObject();

	// nfc
	auto nfcController = GetNfcControllerInstance();
	nfcController->registerCallback(&callback);

	while(1)
	{
		if(get_board_switch_status_with_debounce() == PRESSED)
		{
			nfcController->switchGreenLED(ON);
			delay_ms(2000, MCU_CLOCK);
			nfcController->switchBlueLED(ON);
			delay_ms(2000, MCU_CLOCK);
			nfcController->switchYellowLED(ON);
			delay_ms(3000, MCU_CLOCK);
			nfcController->switchGreenLED(OFF);
			nfcController->switchBlueLED(OFF);
			nfcController->switchYellowLED(OFF);
		}

		if(responseIndicator != 0)
		{
			serialLog->write("Nfc data received!\r\n");

			switch(responseIndicator)
			{
			case 1:
				nfcController->writeData("hello back");
				break;
			case 2:
				nfcController->writeData("Who are you?");
				break;
			default:
				break;
			}

			responseIndicator = 0;
		}
	}
}
