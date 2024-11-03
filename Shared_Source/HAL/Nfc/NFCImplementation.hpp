#ifndef NFCIMPLEMENTATION_HPP
#define NFCIMPLEMENTATION_HPP

#include <NfcInterface.hpp>

class CNfcController : public INfcController
{
public:
	static CNfcController * GetInstance();

	void writeData(const char * data);
	void registerCallback(INfcCallback * callback);

	void switchGreenLED(uint32_t state);
	void switchBlueLED(uint32_t state);
	void switchYellowLED(uint32_t state);

	static void nfcDataReceivedCallback();

private:
	CNfcController();

	INfcCallback * callback;

	static bool nfcInstanceCreated;
	static CNfcController * nfcInstance;
};

#endif // NFCIMPLEMENTATION_HPP
