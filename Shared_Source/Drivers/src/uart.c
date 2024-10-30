#include "uart.h"

#include <stm32f4xx_tools.h>

#define	UART1EN		(1U<<4)
#define UART2EN		(1U<<17)
#define UART3EN		(1U<<18)
#define UART4EN		(1U<<19)
#define UART5EN		(1U<<20)
#define UART6EN		(1U<<5)

#define	CR1_TE		(1U<<3)
#define	CR1_RE		(1U<<2)

#define	CR1_UE		(1U<<13)
#define	SR_TXE		(1U<<7)
#define	SR_RXNE		(1U<<5)

#define	CR1_RXNEIE	(1U<<5)

typedef struct
{
	uint32_t uartNumber;
	void (*uart_rx_callback)(char data, uint32_t uartNumber);

}UARTStorageObject;

static void config_pin_from_uart_number(uint32_t uartNumber);
static void reset_pin_from_uart_number(uint32_t uartNumber);
static void enable_uart_clock_access_from_uart_number(uint32_t uartNumber);
static void disable_uart_clock_access_from_uart_number(uint32_t uartNumber);
static USART_TypeDef * get_uart_mod_from_number(uint32_t uartNumber);
static void uart_write_ch(USART_TypeDef * USARTx, uint32_t ch);
static void uart_set_baudrate(USART_TypeDef *USARTx, uint32_t periphClk, uint32_t baudRate);
static uint16_t compute_uart_bd(uint32_t periphClk, uint32_t baudRate);
static IRQn_Type interrupt_vector_from_uart_number(uint32_t uartNumber);
static char uart_read(USART_TypeDef * USARTx);

static UARTStorageObject uartStorage[6];
static uint32_t uartStorageInitialized = 0;
static uint32_t uartPrintfRerouteNumber = 2;

static uart_result_t validate_uart_object(UARTObject * self)
{
	if(self == NULL)
	{
		return UART_ERROR_INVALID_ARGUMENT;
	}
	if(self->apbClockFrequency == 0 || self->baudRate == 0)
	{
		return UART_ERROR_INVALID_ARGUMENT;
	}
	if(self->uartNumber < 1 || self->uartNumber > 6)
	{
		return UART_ERROR_INVALID_ARGUMENT;
	}
	if(self->transferDirection > UART_TRANSFER_DIR_RXTX)
	{
		return UART_ERROR_INVALID_ARGUMENT;
	}
	return UART_OK;
}

uart_result_t uart_init(UARTObject * self)
{
	if(validate_uart_object(self) != UART_OK)
	{
		return UART_ERROR_INVALID_ARGUMENT;
	}

	if(uartStorageInitialized == 0)
	{
		for(uint32_t i = 0; i < 6; i++)
		{
			uartStorage[i].uartNumber = 0;
			uartStorage[i].uart_rx_callback = NULL;
		}
		uartStorageInitialized = 1;
	}

	USART_TypeDef * USARTx =
			get_uart_mod_from_number(self->uartNumber);

	config_pin_from_uart_number(self->uartNumber);
	enable_uart_clock_access_from_uart_number(self->uartNumber);
	uart_set_baudrate(USARTx, self->apbClockFrequency, self->baudRate);

	if(self->transferDirection == UART_TRANSFER_DIR_TX)
	{
		USARTx->CR1 |= (CR1_TE);
	}
	else if(self->transferDirection == UART_TRANSFER_DIR_RX)
	{
		USARTx->CR1 |= (CR1_RE);
	}
	else if(self->transferDirection == UART_TRANSFER_DIR_RXTX)
	{
		USARTx->CR1 |= ((CR1_TE) | (CR1_RE));
	}

	// enable interrupts
	if(self->transferDirection == UART_TRANSFER_DIR_RX || self->transferDirection == UART_TRANSFER_DIR_RXTX)
	{
		USARTx->CR1 |= (CR1_RXNEIE);

		NVIC_EnableIRQ(
			interrupt_vector_from_uart_number(self->uartNumber)
		);
	}

	// store uart callback
	if(self->uart_reception_callback != NULL)
	{
		for(uint32_t i = 0; i < 6; i++)
		{
			if(uartStorage[i].uart_rx_callback == NULL)
			{
				uartStorage[i].uartNumber = self->uartNumber;
				uartStorage[i].uart_rx_callback = self->uart_reception_callback;
				break;
			}
		}
	}

	// uart enable
	USARTx->CR1 |= (CR1_UE);

	return UART_OK;
}

uart_result_t uart_deinit(UARTObject * self)
{
	if(validate_uart_object(self) != UART_OK)
	{
		return UART_ERROR_INVALID_ARGUMENT;
	}

	USART_TypeDef * USARTx =
			get_uart_mod_from_number(self->uartNumber);

	NVIC_DisableIRQ(
		interrupt_vector_from_uart_number(self->uartNumber)
	);

	// reset control register
	USARTx->CR1 = 0;

	disable_uart_clock_access_from_uart_number(self->uartNumber);
	reset_pin_from_uart_number(self->uartNumber);

	// delete uart callback
	for(uint32_t i = 0; i < 6; i++)
	{
		if(uartStorage[i].uartNumber == self->uartNumber)
		{
			uartStorage[i].uartNumber = 0;
			uartStorage[i].uart_rx_callback = NULL;
			break;
		}
	}
	return UART_OK;
}

uart_result_t uart_write(UARTObject * self, const char * data, uint32_t length)
{
	if(self == NULL || data == NULL)
	{
		return UART_ERROR_INVALID_ARGUMENT;
	}
	else
	{
		USART_TypeDef * USARTx =
				get_uart_mod_from_number(self->uartNumber);

		if(USARTx == NULL)
		{
			return UART_ERROR_INVALID_ARGUMENT;
		}

		uint32_t index = 0;

		while(index < length)
		{
			if(data[index] == '\0')
			{
				break;
			}
			uart_write_ch(USARTx, ((uint32_t)data[index]));
			index++;
		}
		return UART_OK;
	}
}

static void uart_write_ch(USART_TypeDef * USARTx, uint32_t ch)
{
	// make sure the transmit data register is empty
	while(!(USARTx->SR & (SR_TXE))){}
	// write to transmit data register
	USARTx->DR = (ch & 0xFF);
}

static void config_pin_from_uart_number(uint32_t uartNumber)
{
	if(uartNumber == 1){
		gpio_config_port_pin(GPIOA, 9, GPIO_MODE_ALTERNATE);
		gpio_config_port_pin(GPIOA, 10, GPIO_MODE_ALTERNATE);
		gpio_set_alternate_function_to_pin(GPIOA, 9, 7);
		gpio_set_alternate_function_to_pin(GPIOA, 10, 7);
	}
	else if(uartNumber == 2){
		gpio_config_port_pin(GPIOA, 2, GPIO_MODE_ALTERNATE);
		gpio_config_port_pin(GPIOA, 3, GPIO_MODE_ALTERNATE);
		gpio_set_alternate_function_to_pin(GPIOA, 2, 7);
		gpio_set_alternate_function_to_pin(GPIOA, 3, 7);
	}
	else if(uartNumber == 3){
		gpio_config_port_pin(GPIOB, 10, GPIO_MODE_ALTERNATE);
		gpio_config_port_pin(GPIOB, 11, GPIO_MODE_ALTERNATE);
		gpio_set_alternate_function_to_pin(GPIOB, 10, 7);
		gpio_set_alternate_function_to_pin(GPIOB, 11, 7);
	}
	else if(uartNumber == 4){
		gpio_config_port_pin(GPIOC, 10, GPIO_MODE_ALTERNATE);
		gpio_config_port_pin(GPIOC, 11, GPIO_MODE_ALTERNATE);
		gpio_set_alternate_function_to_pin(GPIOC, 10, 8);
		gpio_set_alternate_function_to_pin(GPIOC, 11, 8);
	}
	else if(uartNumber == 5){
		gpio_config_port_pin(GPIOC, 12, GPIO_MODE_ALTERNATE);
		gpio_config_port_pin(GPIOD, 2, GPIO_MODE_ALTERNATE);
		gpio_set_alternate_function_to_pin(GPIOC, 12, 8);
		gpio_set_alternate_function_to_pin(GPIOD, 2, 8);
	}
	else if(uartNumber == 6){
		gpio_config_port_pin(GPIOC, 6, GPIO_MODE_ALTERNATE);
		gpio_config_port_pin(GPIOC, 7, GPIO_MODE_ALTERNATE);
		gpio_set_alternate_function_to_pin(GPIOC, 6, 8);
		gpio_set_alternate_function_to_pin(GPIOC, 7, 8);
	}
}

static void reset_pin_from_uart_number(uint32_t uartNumber)
{
	if(uartNumber == 1){
		gpio_config_port_pin(GPIOA, 9, GPIO_MODE_RESET_STATE);
		gpio_config_port_pin(GPIOA, 10, GPIO_MODE_RESET_STATE);
		gpio_set_alternate_function_to_pin(GPIOA, 9, 0);
		gpio_set_alternate_function_to_pin(GPIOA, 10, 0);
	}
	else if(uartNumber == 2){
		gpio_config_port_pin(GPIOA, 2, GPIO_MODE_RESET_STATE);
		gpio_config_port_pin(GPIOA, 3, GPIO_MODE_RESET_STATE);
		gpio_set_alternate_function_to_pin(GPIOA, 2, 0);
		gpio_set_alternate_function_to_pin(GPIOA, 3, 0);
	}
	else if(uartNumber == 3){
		gpio_config_port_pin(GPIOB, 10, GPIO_MODE_RESET_STATE);
		gpio_config_port_pin(GPIOB, 11, GPIO_MODE_RESET_STATE);
		gpio_set_alternate_function_to_pin(GPIOB, 10, 0);
		gpio_set_alternate_function_to_pin(GPIOB, 11, 0);
	}
	else if(uartNumber == 4){
		gpio_config_port_pin(GPIOC, 10, GPIO_MODE_RESET_STATE);
		gpio_config_port_pin(GPIOC, 11, GPIO_MODE_RESET_STATE);
		gpio_set_alternate_function_to_pin(GPIOC, 10, 0);
		gpio_set_alternate_function_to_pin(GPIOC, 11, 0);
	}
	else if(uartNumber == 5){
		gpio_config_port_pin(GPIOC, 12, GPIO_MODE_RESET_STATE);
		gpio_config_port_pin(GPIOD, 2, GPIO_MODE_RESET_STATE);
		gpio_set_alternate_function_to_pin(GPIOC, 12, 0);
		gpio_set_alternate_function_to_pin(GPIOD, 2, 0);
	}
	else if(uartNumber == 6){
		gpio_config_port_pin(GPIOC, 6, GPIO_MODE_RESET_STATE);
		gpio_config_port_pin(GPIOC, 7, GPIO_MODE_RESET_STATE);
		gpio_set_alternate_function_to_pin(GPIOC, 6, 0);
		gpio_set_alternate_function_to_pin(GPIOC, 7, 0);
	}
}

static void enable_uart_clock_access_from_uart_number(uint32_t uartNumber)
{
	if(uartNumber == 1){
		RCC->APB2ENR |= (UART1EN);
	}
	else if(uartNumber == 2){
		RCC->APB1ENR |= (UART2EN);
	}
	else if(uartNumber == 3){
		RCC->APB1ENR |= (UART3EN);
	}
	else if(uartNumber == 4){
		RCC->APB1ENR |= (UART4EN);
	}
	else if(uartNumber == 5){
		RCC->APB1ENR |= (UART5EN);
	}
	else if(uartNumber == 6){
		RCC->APB2ENR |= (UART6EN);
	}
}

static void disable_uart_clock_access_from_uart_number(uint32_t uartNumber)
{
	if(uartNumber == 1){
		RCC->APB2ENR &= ~(UART1EN);
	}
	else if(uartNumber == 2){
		RCC->APB1ENR &= ~(UART2EN);
	}
	else if(uartNumber == 3){
		RCC->APB1ENR &= ~(UART3EN);
	}
	else if(uartNumber == 4){
		RCC->APB1ENR &= ~(UART4EN);
	}
	else if(uartNumber == 5){
		RCC->APB1ENR &= ~(UART5EN);
	}
	else if(uartNumber == 6){
		RCC->APB2ENR &= ~(UART6EN);
	}
}

static USART_TypeDef * get_uart_mod_from_number(uint32_t uartNumber)
{
	switch(uartNumber)
	{
	case 1:
		return USART1;
	case 2:
		return USART2;
	case 3:
		return USART3;
	case 4:
		return UART4;
	case 5:
		return UART5;
	case 6:
		return USART6;
	default:
		return NULL;
	}
}

static IRQn_Type interrupt_vector_from_uart_number(uint32_t uartNumber)
{
	switch(uartNumber)
	{
	case 1:
		return USART1_IRQn;
	case 2:
		return USART2_IRQn;
	case 3:
		return USART3_IRQn;
	case 4:
		return UART4_IRQn;
	case 5:
		return UART5_IRQn;
	case 6:
		return USART6_IRQn;
	default:
		return 100;
	}
}

uart_result_t uart_route_printf_to(uint32_t uartNumber)
{
	if(uartNumber < 1 || uartNumber > 6)
	{
		return UART_ERROR_INVALID_ARGUMENT;
	}
	uartPrintfRerouteNumber = uartNumber;
	return UART_OK;
}

static char uart_read(USART_TypeDef * USARTx)
{
	// make sure the receiver data register is not empty
	while(!(USARTx->SR & (SR_RXNE))){}
	// read data
	return USARTx->DR;
}

static void uart_set_baudrate(USART_TypeDef *USARTx, uint32_t periphClk, uint32_t baudRate)
{
	USARTx->BRR = compute_uart_bd(periphClk, baudRate);
}

static uint16_t compute_uart_bd(uint32_t periphClk, uint32_t baudRate)
{
	return ((periphClk + (baudRate/2U))/baudRate);
}

int __io_putchar(int ch)
{
	USART_TypeDef * USARTx =
			get_uart_mod_from_number(uartPrintfRerouteNumber);

	uart_write_ch(USARTx, (uint32_t)ch);
	return ch;
}

static void invoke_respective_callback(uint32_t cbNumber, char rx)
{
	for(uint32_t i = 0; i < 6; i++)
	{
		if(uartStorage[i].uartNumber == cbNumber)
		{
			uartStorage[i].uart_rx_callback(rx, cbNumber);
			break;
		}
	}
}

void USART1_IRQHandler()
{
	char rx_char = uart_read(USART1);
	invoke_respective_callback(1, rx_char);
}

void USART2_IRQHandler()
{
	char rx_char = uart_read(USART2);
	invoke_respective_callback(2, rx_char);
}

void USART3_IRQHandler()
{
	char rx_char = uart_read(USART3);
	invoke_respective_callback(3, rx_char);
}

void UART4_IRQHandler()
{
	char rx_char = uart_read(UART4);
	invoke_respective_callback(4, rx_char);
}

void UART5_IRQHandler()
{
	char rx_char = uart_read(UART5);
	invoke_respective_callback(5, rx_char);
}

void USART6_IRQHandler()
{
	char rx_char = uart_read(USART6);
	invoke_respective_callback(6, rx_char);
}






