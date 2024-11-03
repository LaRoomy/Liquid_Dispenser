#ifndef SERIALINTERFACE_HPP_
#define SERIALINTERFACE_HPP_

#include <stdint.h>

class ISerialCallback
{
public:
	virtual ~ISerialCallback() = default;
	virtual void dataReceived(const char * data) = 0;
};

class ISerial
{
public:
	virtual ~ISerial() = default;
	virtual void write(const char * data) = 0;
	virtual void registerCallback(ISerialCallback * callback) = 0;
};

/*
 * Get the default serial port - baudrate defaults to 115200
 */
ISerial * CreateSerialObject();

/*
 * Get a specific serial port - baudrate defaults to 115200
 */
ISerial * CreateSerialObject(uint32_t uartNumber);

/*
 * Get a specific serial port with a specific baudrate setting
 */
ISerial * CreateSerialObject(uint32_t uartNumber, uint32_t baudRate);

void ReleaseSerialObject(ISerial ** ppSerial);

#endif /* SERIALINTERFACE_HPP_ */
