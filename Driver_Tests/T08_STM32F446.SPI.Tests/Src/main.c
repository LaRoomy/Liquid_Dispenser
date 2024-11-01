#include "main.h"
#include "adxl345.h"

/*
 * Test 8 - SPI
 * A simple adxl345 accelerometer board is used with the spi for testing purposes.
 * The user push button is used to start and stop the spi communication to the
 * adxl345. Values (and if it works) can be observed via debug live expressions.
 * */

int16_t x,y,z;
float xg, yg, zg;
uint8_t data_rec[6];
uint32_t run = 0;

int main(void)
{
	CPU_Clock_Config();

	init_board_led();
	init_board_switch();

	control_board_led(ON);

	adxl_init();

	while(1)
	{
		if(get_board_switch_status_with_debounce() == PRESSED)
		{
			run = (run == 1) ? 0 : 1;
		}
		if(run == 1)
		{
			adxl_read(DATA_START_ADDR, data_rec, 6);

			x = ((data_rec[1]<<8)|(data_rec[0]));
			y = ((data_rec[3]<<8)|(data_rec[2]));
			z = ((data_rec[4]<<8)|(data_rec[4]));

			xg = (x * 0.0078);// four g scale factor
			yg = (y * 0.0078);
			zg = (z * 0.0078);

			delay_ms(100, SYS_FRQ);
		}
	}
}




