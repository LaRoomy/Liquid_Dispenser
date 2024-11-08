#ifndef MAIN_HPP_
#define MAIN_HPP_

#include <stm32f4xx.h>
#include <cpu_clock.h>
#include <stm32f4xx_tools.h>
#include <nucleoF446_io.h>

#include <uint8_tools.h>
#include <stdio.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include <Application.hpp>

#include <NfcInterface.hpp>
#include <CanInterface.hpp>
#include <LoggerInterface.hpp>

#include <SerialInterface.hpp>

//#define	DWT_CTRL	(*(volatile uint32_t*)0xE0001000)

#define	CAN_TEST_ID		0xD2U

#endif /* MAIN_HPP_ */
