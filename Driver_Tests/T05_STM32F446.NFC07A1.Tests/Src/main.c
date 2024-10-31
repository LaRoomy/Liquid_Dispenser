#include "main.h"

/*
 * Test 5 - NFC07A1 / ST25DVxxKC
 *
 * When the user button is pressed the NFC07A1 board LEDs are switch from top to ground
 * to check if the output works. After that the result of the nfc07a1 config function is
 * evaluated.
 *
 * If the configuration of the NFC07A1 fails, the nucleo board led is switched on.
 * If the GPO interrupt gets triggered the green led of the nfc07a1 indicates that for 3 seconds.
 *
 * To test the nfc connection the respective android app is necessary: https://github.com/LaRoomy/Android_NFC_Control
 * - the entered data inside of the app will be transmitted via nfc and printed through the uart for evaluation
 * - when the GPO gets triggered, a response is sent and will be printed in the notification area of the app
 *
 * */

uint8_t gpo_activated = 0;
uint8_t nfc_data_received = 0;

void timer_callback();
void nfc_data_received_callback();

TimerObject timerObj =
{
	.systemFrequency = SYS_FRQ,
	.millisecondRate = 1000,
	.timerNumber = 2,
	.timer_callback = timer_callback
};

NFC07A1Object nfc07a1Obj =
{
	.apbClockFrequency = SYS_FRQ,
	.gpoInterruptPin = 6,
	.gpoInterruptPort = GPIOA,
	.i2cNumber = 1,
	.led1Pin = 4,
	.led1Port = GPIOB,
	.led2Pin = 5,
	.led2Port = GPIOB,
	.led3Pin = 10,
	.led3Port = GPIOA,
	.nfc_data_received_irq = nfc_data_received_callback
};

int main(void)
{
	CPU_Clock_Config();

	init_board_led();
	init_board_switch();
	init_test_tools(SYS_FRQ);
	tim_interrupt_init(&timerObj);

	nfc07a1_result_t result = nfc07a1_init(&nfc07a1Obj);
	if(result != NFC07A1_OK)
	{
		control_board_led(ON);
	}

	while(1)
	{
		if(get_board_switch_status_with_debounce() == PRESSED)
		{
			nfc07a1_switch_user_led_1(&nfc07a1Obj, ON);

			delay_ms(1000, SYS_FRQ);

			nfc07a1_switch_user_led_2(&nfc07a1Obj, ON);

			delay_ms(1000, SYS_FRQ);

			nfc07a1_switch_user_led_3(&nfc07a1Obj, ON);

			delay_ms(2000, SYS_FRQ);

			nfc07a1_switch_user_led_1(&nfc07a1Obj, OFF);
			nfc07a1_switch_user_led_2(&nfc07a1Obj, OFF);
			nfc07a1_switch_user_led_3(&nfc07a1Obj, OFF);

			assert_uint_are_equal(NFC07A1_OK, result, "Result of the nfc07a1 config function.");
		}

		if(nfc_data_received == 1)
		{
			nfc_data_received = 0;

			nfc07a1_read_mailbox(&nfc07a1Obj);

			printf("NFC received data:\r\n");
			printf(nfc07a1Obj.mailbox_buffer);
			printf("\r\n\r\n");

			nfc07a1_write_mailbox(&nfc07a1Obj, "Operation success\0", 18);
		}
	}
}

void timer_callback()
{
	if(gpo_activated > 0)
	{
		gpo_activated++;

		if(gpo_activated >= 3)
		{
			gpo_activated = 0;
			nfc07a1_switch_user_led_1(&nfc07a1Obj, OFF);
		}
	}
}

void nfc_data_received_callback()
{
	nfc07a1_switch_user_led_1(&nfc07a1Obj, ON);
	gpo_activated = 1;
	nfc_data_received = 1;
}



