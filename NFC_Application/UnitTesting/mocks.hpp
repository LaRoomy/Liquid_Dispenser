#ifndef MOCKS_HPP_
#define MOCKS_HPP_

#include "gmock/gmock.h"
#include <Application.hpp>
#include <CanInterface.hpp>
#include <NFCInterface.hpp>
#include <LoggerInterface.hpp>

class ApplicationCallbackMock : public IApplicationCallback
{
public:
	MOCK_METHOD(void, enqueueCanData, (void * data), (override));
	MOCK_METHOD(void, enqueueNfcData, (void * data), (override));
	MOCK_METHOD(void, onCriticalError, (const char * msg), (override));
};

class CanInterfaceMock : public ICanController
{
public:
	MOCK_METHOD(void, init, (PCAN_FILTERS pFilters), (override));
	MOCK_METHOD(void, deInit, (), (override));
	MOCK_METHOD(void, writeData, (PCAN_DATA pCanData), (override));
	MOCK_METHOD(void, enableLoopBackMode, (), (override));
	MOCK_METHOD(void, convertCanDataToCharArray, (PCAN_DATA canData, char * cArr), (override));
	MOCK_METHOD(void, convertCharArrayToCanUint8Array, (const char * cArr, uint8_t * out, uint32_t size), (override));

	void registerCallback(ICanCallback * callback)
	{
		this->cb = callback;
	}
	void triggerCanCallbackDataReceived(PCAN_DATA pData)
	{
		if(this->cb != nullptr)
		{
			this->cb->canDataReceived(pData);
		}
	}

private:
	ICanCallback * cb = nullptr;
};

class NfcInterfaceMock : public INfcController
{
public:
	MOCK_METHOD(void, writeData, (const char * data), (override));
	MOCK_METHOD(void, switchGreenLED, (uint32_t state), (override));
	MOCK_METHOD(void, switchBlueLED, (uint32_t state), (override));
	MOCK_METHOD(void, switchYellowLED, (uint32_t state), (override));

	void registerCallback(INfcCallback * callback)
	{
		this->cb = callback;
	}
	void triggerNfcCallbackDataReceived(const char * data)
	{
		if(this->cb != nullptr)
		{
			this->cb->nfcDataReceived(data);
		}
	}

private:
	INfcCallback * cb = nullptr;
};

class LoggerMock : public ILogger
{
public:
	MOCK_METHOD(void, LogString, (const char * str), (override));
	MOCK_METHOD(void, LogStringLn, (const char * str), (override));
};

#endif /* MOCKS_HPP_ */
