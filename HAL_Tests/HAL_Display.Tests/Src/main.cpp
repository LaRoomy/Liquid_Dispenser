#include "main.hpp"

uint32_t testCounter;

void ShowStartSequence(IDisplayController * displayController);
void ExecuteTests(IDisplayController * displayController);

void FontTest(IDisplayController * displayController);
void DrawRectTest(IDisplayController * displayController);
void FillRectTest(IDisplayController * displayController);
void ColorTest(IDisplayController * displayController);
void DrawPixelTest(IDisplayController * displayController);
void DrawImageTest(IDisplayController * displayController);

int main(void)
{
	CPU_Clock_Config();
	init_board_switch();

	testCounter = 0;

	auto displayController = GetDisplayControllerInstance();
	ShowStartSequence(displayController);

	while(1)
	{
		if(get_board_switch_status_with_debounce() == PRESSED)
		{
			ExecuteTests(displayController);
		}
	}
}

void ShowStartSequence(IDisplayController * displayController)
{
	POINT textCoordinates = {20,50};
	displayController->fillScreen(Colors::DarkSlateGray);
	displayController->drawText(&textCoordinates, "Press button to execute", Font::Medium, Colors::White, Colors::DarkSlateGray);
	textCoordinates.y = 80;
	displayController->drawText(&textCoordinates, "a test. When a test is", Font::Medium, Colors::White, Colors::DarkSlateGray);
	textCoordinates.y = 110;
	displayController->drawText(&textCoordinates, "finished, the next test", Font::Medium, Colors::White, Colors::DarkSlateGray);
	textCoordinates.y = 140;
	displayController->drawText(&textCoordinates, "will be executed when the", Font::Medium, Colors::White, Colors::DarkSlateGray);
	textCoordinates.y = 170;
	displayController->drawText(&textCoordinates, "button is pressed again.", Font::Medium, Colors::White, Colors::DarkSlateGray);
}

void ExecuteTests(IDisplayController * displayController)
{
	switch(testCounter)
	{
	case 0:
		FontTest(displayController);
		break;
	case 1:
		DrawRectTest(displayController);
		break;
	case 2:
		FillRectTest(displayController);
		break;
	case 3:
		ColorTest(displayController);
		break;
	case 4:
		DrawPixelTest(displayController);
		break;
	case 5:
		DrawImageTest(displayController);
		break;
	default:
		testCounter = 0;
		ShowStartSequence(displayController);
		return;
	}
	testCounter++;
}

void FontTest(IDisplayController * displayController)
{
	POINT textCoordinates = {20,20};
	displayController->fillScreen(Colors::DarkSlateGray);
	displayController->drawText(&textCoordinates, "DrawText Test..", Font::Medium, Colors::White, Colors::DarkSlateGray);
	delay_ms(1000, MCU_CLOCK);
	textCoordinates.x = 40;
	textCoordinates.y = 70;
	displayController->drawText(&textCoordinates, "Large Font!", Font::Large, Colors::White, Colors::DarkSlateGray);
	delay_ms(1000, MCU_CLOCK);
	textCoordinates.y = 120;
	displayController->drawText(&textCoordinates, "Medium Font!", Font::Medium, Colors::White, Colors::DarkSlateGray);
	delay_ms(1000, MCU_CLOCK);
	textCoordinates.y = 170;
	displayController->drawText(&textCoordinates, "Small Font!", Font::Small, Colors::White, Colors::DarkSlateGray);
}

void DrawRectTest(IDisplayController * displayController)
{
	POINT textCoordinates = {20,20};
	displayController->fillScreen(Colors::Black);
	displayController->drawText(&textCoordinates, "DrawRect Test..", Font::Medium, Colors::White, Colors::Black);

	RECT rc;
	rc.x = 20;
	rc.y = 100;
	rc.cx = 50;
	rc.cy = 50;
	displayController->drawRect(&rc, Colors::LightCyan);

	delay_ms(1000, MCU_CLOCK);

	rc.x = 150;
	rc.y = 120;
	rc.cx = 169;
	rc.cy = 119;
	displayController->drawRect(&rc, Colors::LightGreen);
}

void FillRectTest(IDisplayController * displayController)
{
	POINT textCoordinates = {20,20};
	displayController->fillScreen(Colors::Black);
	displayController->drawText(&textCoordinates, "FillRect Test..", Font::Medium, Colors::White, Colors::Black);

	RECT rc;
	rc.x = 0;
	rc.y = 100;
	rc.cx = 80;
	rc.cy = 140;
	displayController->fillRect(&rc, Colors::Gold);

	delay_ms(1000, MCU_CLOCK);

	rc.x = 80;
	displayController->fillRect(&rc, Colors::Pink);

	delay_ms(1000, MCU_CLOCK);

	rc.x = 160;
	displayController->fillRect(&rc, Colors::YellowGreen);

	delay_ms(1000, MCU_CLOCK);

	rc.x = 240;
	displayController->fillRect(&rc, Colors::Coral);
}

void ColorTest(IDisplayController * displayController)
{
	POINT textRC = {20,20};
	displayController->fillScreen(Colors::Black);
	displayController->drawText(&textRC, "Color Test..", Font::Medium, Colors::White, Colors::Black);

	RECT rc;
	rc.x = 0;
	rc.y = 80;
	rc.cx = 320;
	rc.cy = 40;
	displayController->fillRect(&rc, Colors::Gold);
	textRC.x = rc.x + 20;
	textRC.y = rc.y + 4;
	displayController->drawText(&textRC, "Gold", Font::Medium, Colors::Black, Colors::Gold);

	rc.y = 120;
	displayController->fillRect(&rc, Colors::Pink);
	textRC.x = rc.x + 20;
	textRC.y = rc.y + 4;
	displayController->drawText(&textRC, "Pink", Font::Medium, Colors::Black, Colors::Pink);

	rc.y = 160;
	displayController->fillRect(&rc, Colors::YellowGreen);
	textRC.x = rc.x + 20;
	textRC.y = rc.y + 4;
	displayController->drawText(&textRC, "YellowGreen", Font::Medium, Colors::Black, Colors::YellowGreen);

	rc.y = 200;
	displayController->fillRect(&rc, Colors::Coral);
	textRC.x = rc.x + 20;
	textRC.y = rc.y + 4;
	displayController->drawText(&textRC, "Coral", Font::Medium, Colors::Black, Colors::Coral);
}

void DrawPixelTest(IDisplayController * displayController)
{
	POINT textCoordinates = {20,20};
	displayController->fillScreen(Colors::Black);
	displayController->drawText(&textCoordinates, "DrawPixel Test..", Font::Medium, Colors::White, Colors::Black);

	POINT pixelPoint;
	pixelPoint.x = 20;
	pixelPoint.y = 100;

	for(uint32_t i = 0; i <= 100; i++)
	{
		pixelPoint.x = 20 + i;
		displayController->drawPixel(&pixelPoint, Colors::White);
	}

	pixelPoint.y = 150;

	for(uint32_t i = 100; i > 0; i--)
	{
		pixelPoint.x = 20 + i;
		displayController->drawPixel(&pixelPoint, Colors::White);
	}
}

void DrawImageTest(IDisplayController * displayController)
{
	POINT textCoordinates = {20,20};
	displayController->fillScreen(Colors::Black);
	displayController->drawText(&textCoordinates, "DrawImage Test..", Font::Medium, Colors::White, Colors::Black);
	delay_ms(1500, MCU_CLOCK);

	RECT fillRect =
	{
		.x = 0,
		.y = 0,
		.cx = 320,
		.cy = 240
	};

	displayController->fillRect(&fillRect, Colors::Black);

	IMAGE waterDropImage =
	{
		.imageData = (const uint16_t *)img_water_drops,
		.pos_x = 0,
		.pos_y = 40,
		.width = 160,
		.heigth = 160,
	};
	displayController->drawImage(&waterDropImage);

	IMAGE stopSignImage =
	{
		.imageData = (const uint16_t *)img_stop_sign,
		.pos_x = 170,
		.pos_y = 50,
		.width = 140,
		.heigth = 140
	};
	displayController->drawImage(&stopSignImage);
}





