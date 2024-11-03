#ifndef NFCINTERFACE_HPP
#define	NFCINTERFACE_HPP

#include <stdint.h>

class INfcCallback
{
public:
	virtual ~INfcCallback() = default;
	virtual void nfcDataReceived(const char * data) = 0;
};

class INfcController
{
public:
	virtual ~INfcController() = default;
	virtual void writeData(const char * data) = 0;
	virtual void registerCallback(INfcCallback * callback) = 0;

	virtual void switchGreenLED(uint32_t state) = 0;
	virtual void switchBlueLED(uint32_t state) = 0;
	virtual void switchYellowLED(uint32_t state) = 0;
};

/*
 * Create or get the NFC controller singleton object
 */
INfcController * GetNfcControllerInstance();

#endif // NFCINTERFACE_HPP
