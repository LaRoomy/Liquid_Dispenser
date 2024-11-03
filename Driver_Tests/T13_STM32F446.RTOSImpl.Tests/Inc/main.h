#ifndef MAIN_H_
#define MAIN_H_

#include <stm32f4xx.h>
#include <cpu_clock.h>
#include <stm32f4xx_tools.h>

#include <FreeRTOS.h>
#include <task.h>

#define	DWT_CTRL	(*(volatile uint32_t*)0xE0001000)

#define	GREEN_LED_PIN	4
#define	BLUE_LED_PIN	5

#endif /* MAIN_H_ */
