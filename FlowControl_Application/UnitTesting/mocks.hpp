#ifndef MOCKS_HPP_
#define MOCKS_HPP_

#include "gmock/gmock.h"
#include <Application.hpp>
#include <CanInterface.hpp>
#include <FlowControlInterface.hpp>
#include <LoggerInterface.hpp>

class ApplicationCallbackMock : public IApplicationCallback
{
public:
	MOCK_METHOD(void, enqueueCanData, (void * data), (override));
	MOCK_METHOD(void, startFlowOperationTask, (), (override));
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

class FlowControllCallbackMock : public IFlowControlCallback
{
public:
	MOCK_METHOD(void, flowStopped, (), (override));
	MOCK_METHOD(void, containerEmpty, (), (override));
};

class FlowControllerMock : public IFlowController
{
public:
	MOCK_METHOD(void, startWaterFlow, (uint32_t quantity_dl), (override));
	MOCK_METHOD(void, stopWaterFlow, (), (override));
	MOCK_METHOD(bool, isFlowActive, (), (override));
	MOCK_METHOD(void, registerCallback, (IFlowControlCallback * callback), (override));
};

class LoggerMock : public ILogger
{
public:
	MOCK_METHOD(void, LogString, (const char * str), (override));
	MOCK_METHOD(void, LogStringLn, (const char * str), (override));
};

#endif /* MOCKS_HPP_ */











