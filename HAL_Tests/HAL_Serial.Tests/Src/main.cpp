#include "main.hpp"

class SerialCallback : public ISerialCallback
{
public:
	void dataReceived(const char * data) override
	{
		if(data != nullptr)
		{
			if(strcmp(data, "hello led\n") == 0)
			{
				control_board_led(ON);
			}
			else if(strcmp(data, "ciao led\n") == 0)
			{
				control_board_led(OFF);
			}
		}
	}
};

SerialCallback callback;

int main(void)
{
	CPU_Clock_Config();
	init_board_led();
	init_board_switch();


	auto serialObject = CreateSerialObject();

	serialObject->registerCallback(
			dynamic_cast<ISerialCallback*>(&callback)
		);

	while(1)
	{
		if(get_board_switch_status_with_debounce() == PRESSED)
		{
			serialObject->write("hello from the microcontroller\r\n");
		}
	}
}
