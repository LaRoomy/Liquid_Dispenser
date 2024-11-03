#include "SerialInUseManager.hpp"
#include <stddef.h>

bool SerialInUseManager::reserveFreeUartNumber(uint32_t number, void * reference)
{
	if(number > 0 && number < 7)
	{
		uint32_t uIndex = number - 1;

		if(uartInUseRegister[uIndex].state == 1)
		{
			return false;
		}
		else
		{
			uartInUseRegister[uIndex].state = 1;
			uartInUseRegister[uIndex].ref = reference;
			return true;
		}
	}
	return false;
}

void SerialInUseManager::releaseReservedUartNumber(uint32_t uartNumber)
{
	uint32_t uIndex = uartNumber - 1;
	if(uIndex < 6)
	{
		uartInUseRegister[uIndex].state = 0;
		uartInUseRegister[uIndex].ref = NULL;
	}
}

bool SerialInUseManager::isUartAvailable(uint32_t uartNumber)
{
	if(uartNumber < 7 && uartNumber > 0)
	{
		uint32_t uIndex = uartNumber - 1;
		return uartInUseRegister[uIndex].state == 0 ? true : false;
	}
	return false;
}

void* SerialInUseManager::getReferenceForUartNumber(uint32_t uartNumber)
{
	if(uartNumber < 7 && uartNumber > 0)
	{
		uint32_t uIndex = uartNumber - 1;
		return uartInUseRegister[uIndex].ref;
	}
	return nullptr;
}

UART_MANAGE_DATA SerialInUseManager::uartInUseRegister[6] = { {0, NULL}, {0, NULL}, {0, NULL}, {0, NULL}, {0, NULL}, {0, NULL} };
