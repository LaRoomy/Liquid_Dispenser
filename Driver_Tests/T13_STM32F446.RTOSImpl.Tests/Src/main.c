#include "main.h"

uint32_t SystemCoreClock = 48000000U;

/*
 * Test 13 - FreeRTOS implementation
 *
 * This test is only used to verify the FreeRTOS and SEGGER configuration.
 * Two tasks are started which are triggering two external leds in different frequencies
 * on PA6 and PA7 respectively. Additionally SEGGER is started and is recording.
 * The recorded trace can be accessed through the _SEGGER_RTT expression.
 *
 * */

TaskHandle_t ledBlueTaskHandle;
TaskHandle_t ledGreenTaskHandle;

void led_blue_task(void * params);
void led_green_task(void * params);

int main(void)
{
	// enable FPU
	SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));

	BaseType_t status;

	vInitPrioGroupValue();

	CPU_Clock_Config();

	gpio_config_port_pin(GPIOB, GREEN_LED_PIN, GPIO_MODE_OUTPUT);
	gpio_config_port_pin(GPIOB, BLUE_LED_PIN, GPIO_MODE_OUTPUT);

	//enable the CYCCNT counter
	DWT_CTRL |= (1U<<0);

	SEGGER_SYSVIEW_Conf();
	SEGGER_SYSVIEW_Start();

	status = xTaskCreate(led_blue_task, "blue task", 200, NULL, 2, &ledBlueTaskHandle);
	configASSERT(status == pdPASS);

	status = xTaskCreate(led_green_task, "white task", 200, NULL, 2, &ledGreenTaskHandle);
	configASSERT(status == pdPASS);

	vTaskStartScheduler();

	while(1)
	{}
}

void led_blue_task(void * params)
{
	while(1)
	{
		gpio_control_pin(GPIOB, BLUE_LED_PIN, TOGGLE);
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

void led_green_task(void * params)
{
	while(1)
	{
		gpio_control_pin(GPIOB, GREEN_LED_PIN, TOGGLE);
		vTaskDelay(pdMS_TO_TICKS(200));
	}
}








