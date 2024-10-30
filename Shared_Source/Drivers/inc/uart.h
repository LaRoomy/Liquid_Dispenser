#ifndef UART_H_
#define UART_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdio.h>
#include <stm32f4xx.h>

#define		UART_TRANSFER_DIR_TX		0U
#define		UART_TRANSFER_DIR_RX		1U
#define		UART_TRANSFER_DIR_RXTX		2U

#define		UART_OK						0
#define		UART_ERROR_INVALID_ARGUMENT	(-22)

typedef struct
{
	uint32_t uartNumber;
	uint32_t apbClockFrequency;
	uint32_t baudRate;
	uint32_t transferDirection;
	void (*uart_reception_callback)(char data, uint32_t uartNumber);

}UARTObject;

typedef int uart_result_t;

uart_result_t uart_init(UARTObject * self);
uart_result_t uart_deinit(UARTObject * self);
uart_result_t uart_write(UARTObject * self, const char * data, uint32_t length);
uart_result_t uart_route_printf_to(uint32_t uartNumber);
/*
 * UART pin definition:
 * --------------------
 * | Number |  TX  |  RX  | Alternate function
 * ---------------------------------------------
 * | UART1  | PA9  | PA10 | AF7
 * | UART2  | PA2  | PA3  | AF7
 * | UART3  | PB10 | PB11 | AF7
 * | UART4  | PC10 | PC11 | AF8
 * | UART5  | PC12 | PD2  | AF8
 * | UART6  | PC6  | PC7  | AF8
 * */
#ifdef __cplusplus
 } // extern "C"
#endif

#endif /* UART_H_ */
