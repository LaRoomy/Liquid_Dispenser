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

#include <CanInterface.hpp>
#include <LoggerInterface.hpp>

#include <SerialInterface.hpp>

#include <can_communication_codes.h>

#define ULONG_MAX 0xffffffff

#endif /* MAIN_HPP_ */
