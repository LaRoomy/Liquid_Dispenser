#ifndef SERIALIMPLEMENTATION_HPP_
#define SERIALIMPLEMENTATION_HPP_

#include <SerialInterface.hpp>
#include <uart.h>

class CSerial : public ISerial
{
public:
	CSerial(uint32_t uartNumber, uint32_t baudRate);
	~CSerial();

	void write(const char * data) override;
	void registerCallback(ISerialCallback * callback) override;
private:
	UARTObject uartObject;
	ISerialCallback * sCallback;
	char inputBuffer[256];
	uint32_t inputBufferCounter;

	static void uartCallback(char data, uint32_t uartNumber);
	void resetInputBuffer();
	void triggerCallbackIfApplicable();
};




#endif /* SERIALIMPLEMENTATION_HPP_ */
