#include "main.hpp"

bool touchTrackingStarted;

void logCoordinates(ISerial * serial, uint16_t x, uint16_t y);

int main(void)
{
	CPU_Clock_Config();
	init_board_switch();

	touchTrackingStarted = false;

	// serial log
	auto serialObject = CreateSerialObject();

	// touch control
	auto touchController = GetTouchControllerInstance();

	// display
	auto displayController = GetDisplayControllerInstance();

	while(1)
	{
		if(get_board_switch_status_with_debounce() == PRESSED)
		{
			touchTrackingStarted = touchTrackingStarted ? false : true;

			if(touchTrackingStarted)
			{
				serialObject->write("Touch tracking started..\r\n\r\n");
			}
			else
			{
				serialObject->write("\r\nTouch tracking stopped!\r\n\r\n");
				displayController->fillScreen(Colors::Black);
			}
		}
		if(touchTrackingStarted)
		{
			uint16_t x = 0, y = 0;

			if(touchController->getCoordinates(&x, &y) == true)
			{
				POINT pt;
				pt.x = x;
				pt.y = y;
				displayController->drawPixel(&pt, Colors::Cyan);
				logCoordinates(serialObject, x, y);
			}
		}
	}
}

void logCoordinates(ISerial * serial, uint16_t x, uint16_t y)
{
	char buffer[256] = { '\0' };
	sprintf(buffer, "X = %u ; Y = %u\r\n", (unsigned int)x, (unsigned int)y);
	serial->write(buffer);
}




