#ifndef SERIALINUSEMANAGER_HPP_
#define SERIALINUSEMANAGER_HPP_

#include <stdint.h>

typedef struct
{
	uint32_t state;
	void * ref;

}UART_MANAGE_DATA;

class SerialInUseManager
{
public:
	static bool reserveFreeUartNumber(uint32_t uartNumber, void * reference);
	static void releaseReservedUartNumber(uint32_t uartNumber);
	static bool isUartAvailable(uint32_t uartNumber);
	static void* getReferenceForUartNumber(uint32_t uartNumber);
private:
	static UART_MANAGE_DATA uartInUseRegister[6];
};


#endif /* SERIALINUSEMANAGER_HPP_ */
