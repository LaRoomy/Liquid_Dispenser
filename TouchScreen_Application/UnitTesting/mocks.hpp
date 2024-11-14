#ifndef MOCKS_HPP_
#define MOCKS_HPP_

#include "gmock/gmock.h"
#include <Application.hpp>
#include <CanInterface.hpp>
#include <DisplayInterface.hpp>
#include <TouchInterface.hpp>
#include <LoggerInterface.hpp>

class ApplicationCallbackMock : public IApplicationCallback
{
public:
	MOCK_METHOD(void, enqueueDisplayInstruction, (void * instruction), (override));
	MOCK_METHOD(void, enqueueCanData, (void * data), (override));
	MOCK_METHOD(void, handleCriticalError, (const char * msg), (override));
};

class CanControllerMock : public ICanController
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

class DisplayControllerMock : public IDisplayController
{
public:
	MOCK_METHOD(uint32_t, getScreenWidth, (), (override));
	MOCK_METHOD(uint32_t, getScreenHeight, (), (override));

	MOCK_METHOD(void, getTextSize, (const char * text, Font font, PSIZE textSize_out), (override));

	MOCK_METHOD(void, drawImage, (PIMAGE pImage), (override));
	MOCK_METHOD(void, drawText, (PPOINT pCoordinates, const char * text, Font font, const COLOR& foregroundColor, const COLOR& backgroundColor), (override));
	MOCK_METHOD(void, drawPixel, (PPOINT pPoint, const COLOR& color), (override));
	MOCK_METHOD(void, drawRect, (PRECT pRect, const COLOR& color), (override));

	MOCK_METHOD(void, fillRect, (PRECT pRect, const COLOR& color), (override));
	MOCK_METHOD(void, fillScreen, (const COLOR& color), (override));
};

class TouchControllerMock : public ITouchController
{
public:
	MOCK_METHOD(bool, isTouchPressed, (), (override));
	MOCK_METHOD(bool, getCoordinates, (uint16_t * x, uint16_t * y), (override));
};

class LoggerMock : public ILogger
{
public:
	MOCK_METHOD(void, LogString, (const char * str), (override));
	MOCK_METHOD(void, LogStringLn, (const char * str), (override));
};

#endif /* MOCKS_HPP_ */











