#include "SerialImplementation.hpp"
#include "SerialInUseManager.hpp"
#include "string.h"

CSerial::CSerial(uint32_t uartNumber, uint32_t baudRate)
	: sCallback(nullptr)
{
	this->resetInputBuffer();

	if(SerialInUseManager::reserveFreeUartNumber(uartNumber, this))
	{
		this->uartObject.uartNumber = uartNumber;
		this->uartObject.baudRate = baudRate;
		this->uartObject.apbClockFrequency = MCU_CLOCK;
		this->uartObject.transferDirection = UART_TRANSFER_DIR_RXTX;
		this->uartObject.uart_reception_callback = CSerial::uartCallback;

		uart_init(&this->uartObject);
	}
}

CSerial::~CSerial()
{
	uart_deinit(&uartObject);
	SerialInUseManager::releaseReservedUartNumber(this->uartObject.uartNumber);
}

void CSerial::write(const char * data)
{
	uart_write(&uartObject, data, strlen(data));
}

void CSerial:: registerCallback(ISerialCallback * callback)
{
	this->sCallback = callback;
}

void CSerial::uartCallback(char data, uint32_t uartNumber)
{
	CSerial * _this_ =
			reinterpret_cast<CSerial*>(SerialInUseManager::getReferenceForUartNumber(uartNumber));

	if(_this_ != nullptr)
	{
		_this_->inputBuffer[_this_->inputBufferCounter] = data;
		_this_->inputBufferCounter++;

		if(data == '\n' || data == '\r' || data == '\0' || _this_->inputBufferCounter == 255)
		{
			_this_->inputBuffer[_this_->inputBufferCounter] = '\0';
			_this_->triggerCallbackIfApplicable();
			_this_->resetInputBuffer();
		}
	}
}

void CSerial::triggerCallbackIfApplicable()
{
	if(this->sCallback != nullptr)
	{
		this->sCallback->dataReceived(inputBuffer);
	}
}

void CSerial::resetInputBuffer()
{
	memset(this->inputBuffer, 0, sizeof(this->inputBuffer));
	this->inputBufferCounter = 0;
}

// **********************************************************

ISerial * CreateSerialObject()
{
	if(SerialInUseManager::isUartAvailable(2U))
	{
		return new CSerial(2U, 115200U);
	}
	else
	{
		return nullptr;
	}
}

ISerial * CreateSerialObject(uint32_t uartNumber)
{
	if(SerialInUseManager::isUartAvailable(uartNumber))
	{
		return new CSerial(uartNumber, 115200U);
	}
	else
	{
		return nullptr;
	}
}

ISerial * CreateSerialObject(uint32_t uartNumber, uint32_t baudRate)
{
	if(SerialInUseManager::isUartAvailable(uartNumber))
	{
		return new CSerial(uartNumber, baudRate);
	}
	else
	{
		return nullptr;
	}
}
