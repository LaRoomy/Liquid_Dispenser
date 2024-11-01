#include "main.h"

/*
 * Test 8 - ILI9341 Display and touch controller
 * When the board switch is pressed the test sequence is started and all functions of the
 * display are tested. The sequence is self-explanatory.
 * At the end of the sequence the touchscreen is activated and can be tested by drawing
 * something on the display.
 *
 * NOTES:
 * 		- the board led is not usable when spi 1 is enabled (it's the sck pin)
 * */

ILI9341TouchObject iliTouchObj =
{
	.spiNumber = 1,
	.csGPIOPort = GPIOA,
	.csPinNumber = 9,
	.irqGPIOPort = GPIOA,
	.irqPinNumber = 8
};

ILI9341DisplayObject iliDisplayObj =
{
	.spiNumber = 2,
	.csPort = GPIOB,
	.csPinNumber = 1,
	.dcPort = GPIOB,
	.dcPinNumber = 2,
	.resetPort = GPIOB,
	.resetPinNumber = 12,
	.sys_frq = SYS_FRQ
};

void ili_test();

int main(void)
{
	CPU_Clock_Config();

	init_board_switch();

	ILI9341_init(&iliDisplayObj);
	ILI9341_touch_init(&iliTouchObj);

	while(1)
	{
		if(get_board_switch_status_with_debounce() == PRESSED)
		{
			ili_test();
		}
	}
}

void ili_test()
{
	// Check border
	ILI9341_fill_screen(&iliDisplayObj, ILI9341_BLACK);

	for(int x = 0; x < ILI9341_WIDTH; x++) {
		ILI9341_draw_pixel(&iliDisplayObj, x, 0, ILI9341_RED);
		ILI9341_draw_pixel(&iliDisplayObj, x, ILI9341_HEIGHT-1, ILI9341_RED);
	}

	for(int y = 0; y < ILI9341_HEIGHT; y++) {
		ILI9341_draw_pixel(&iliDisplayObj, 0, y, ILI9341_RED);
		ILI9341_draw_pixel(&iliDisplayObj, ILI9341_WIDTH-1, y, ILI9341_RED);
	}

	delay_ms(3000, SYS_FRQ);

	// Check fonts
	ILI9341_fill_screen(&iliDisplayObj, ILI9341_BLACK);
	ILI9341_write_string(&iliDisplayObj, 0, 0, "Font_7x10, red on black, lorem ipsum dolor sit amet", Font_7x10, ILI9341_RED, ILI9341_BLACK);
	ILI9341_write_string(&iliDisplayObj, 0, 3*10, "Font_11x18, green, lorem ipsum dolor sit amet", Font_11x18, ILI9341_GREEN, ILI9341_BLACK);
	ILI9341_write_string(&iliDisplayObj, 0, 3*10+3*18, "Font_16x26, blue, lorem ipsum dolor sit amet", Font_16x26, ILI9341_BLUE, ILI9341_BLACK);

	delay_ms(1000, SYS_FRQ);

	ILI9341_invert_colors(&iliDisplayObj, TRUE);

	delay_ms(1000, SYS_FRQ);

	ILI9341_invert_colors(&iliDisplayObj, FALSE);

	delay_ms(3000, SYS_FRQ);

	// Check colors
	ILI9341_fill_screen(&iliDisplayObj, ILI9341_WHITE);
	ILI9341_write_string(&iliDisplayObj, 0, 0, "WHITE", Font_11x18, ILI9341_BLACK, ILI9341_WHITE);

	delay_ms(500, SYS_FRQ);

	ILI9341_fill_screen(&iliDisplayObj, ILI9341_BLUE);
	ILI9341_write_string(&iliDisplayObj, 0, 0, "BLUE", Font_11x18, ILI9341_BLACK, ILI9341_BLUE);

	delay_ms(500, SYS_FRQ);

	ILI9341_fill_screen(&iliDisplayObj, ILI9341_RED);
	ILI9341_write_string(&iliDisplayObj, 0, 0, "RED", Font_11x18, ILI9341_BLACK, ILI9341_RED);

	delay_ms(500, SYS_FRQ);

	ILI9341_fill_screen(&iliDisplayObj, ILI9341_GREEN);
	ILI9341_write_string(&iliDisplayObj, 0, 0, "GREEN", Font_11x18, ILI9341_BLACK, ILI9341_GREEN);

	delay_ms(500, SYS_FRQ);

	ILI9341_fill_screen(&iliDisplayObj, ILI9341_CYAN);
	ILI9341_write_string(&iliDisplayObj, 0, 0, "CYAN", Font_11x18, ILI9341_BLACK, ILI9341_CYAN);

	delay_ms(500, SYS_FRQ);

	ILI9341_fill_screen(&iliDisplayObj, ILI9341_MAGENTA);
	ILI9341_write_string(&iliDisplayObj, 0, 0, "MAGENTA", Font_11x18, ILI9341_BLACK, ILI9341_MAGENTA);

	delay_ms(500, SYS_FRQ);

	ILI9341_fill_screen(&iliDisplayObj, ILI9341_YELLOW);
	ILI9341_write_string(&iliDisplayObj, 0, 0, "YELLOW", Font_11x18, ILI9341_BLACK, ILI9341_YELLOW);

	delay_ms(500, SYS_FRQ);

	ILI9341_fill_screen(&iliDisplayObj, ILI9341_BLACK);
	ILI9341_write_string(&iliDisplayObj, 0, 0, "BLACK", Font_11x18, ILI9341_WHITE, ILI9341_BLACK);

	delay_ms(500, SYS_FRQ);

	// test image
	ILI9341_draw_image(&iliDisplayObj,0, 0, 320, 240, (const uint16_t*)test_img);

	delay_ms(3000, SYS_FRQ);

	ILI9341_fill_rectangle(&iliDisplayObj, 0, 0, 80, 240, ILI9341_ORANGE);
	ILI9341_fill_rectangle(&iliDisplayObj, 80, 0, 80, 240, ILI9341_MINT);
	ILI9341_fill_rectangle(&iliDisplayObj, 160, 0, 80, 240, ILI9341_PURPLE);
	ILI9341_fill_rectangle(&iliDisplayObj, 240, 0, 80, 240, ILI9341_PINK);

	delay_ms(1000, SYS_FRQ);

	ILI9341_fill_screen(&iliDisplayObj, ILI9341_BLACK);
	ILI9341_write_string(&iliDisplayObj, 0, 0, "Touchpad test.  Draw something!", Font_11x18, ILI9341_WHITE, ILI9341_BLACK);

	delay_ms(1000, SYS_FRQ);

	ILI9341_fill_screen(&iliDisplayObj, ILI9341_BLACK);

	int npoints = 0;

	while(npoints < 10000)
	{
		uint16_t x, y;

		if(ILI9341_touch_get_coordinates(&iliTouchObj, &x, &y, ILI9341_TOUCH_ROTATE_LEFT) == 0)
		{
			ILI9341_draw_pixel(&iliDisplayObj, x, y, ILI9341_WHITE);
			npoints++;
		}
		if(get_board_switch_status() == PRESSED)
		{
			npoints = -1;
			break;
		}
	}
	if(npoints == -1)
	{
		while(get_board_switch_status() == PRESSED);
		delay_ms(50, SYS_FRQ);
	}
	ILI9341_fill_screen(&iliDisplayObj, ILI9341_BLACK);
	ILI9341_write_string(&iliDisplayObj, 10, 10, "End of sequence", Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
	ILI9341_write_string(&iliDisplayObj, 10, 40, "Press to start over..", Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
}




