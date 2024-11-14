#include "gtest/gtest.h"
#include "mocks.hpp"
#include "Application.hpp"
#include <ConversionTool.hpp>
#include <can_communication_codes.h>

using ::testing::_;

TEST(InvocationTests, WrongParameterInvokesCriticalErrorMethod)
{
	ApplicationCallbackMock cbMock;

	auto appInstance = Application::GetInstance();
	appInstance->registerCallback(dynamic_cast<IApplicationCallback*>(&cbMock));

	EXPECT_CALL(cbMock, handleCriticalError(_)).Times(1);

	appInstance->init(nullptr, nullptr, nullptr, nullptr);
}

TEST(InvocationTests, CanReceivedDataAndInvokesEnqueueMethod)
{
	uint8_t testData[] = {1,2,3,4,9,8,7};

	ApplicationCallbackMock cbMock;
	CanControllerMock canCtrlMock;
	DisplayControllerMock displayCtrlMock;
	TouchControllerMock touchCtrlMock;
	LoggerMock loggerMock;

	CAN_DATA cData;
	cData.dataSize = 7U;
	cData.data = testData;
	cData.stdID = 0x01U;

	auto appInstance = Application::GetInstance();
	appInstance->registerCallback(dynamic_cast<IApplicationCallback*>(&cbMock));
	appInstance->init(&canCtrlMock, &displayCtrlMock, &touchCtrlMock, &loggerMock);

	EXPECT_CALL(cbMock, enqueueCanData(_)).Times(1);

	canCtrlMock.triggerCanCallbackDataReceived(&cData);

	// tear down
	appInstance->CleanUp();
}

TEST(InvocationTests, CanDataProcessedAndInvokesDisplayEnqueueMethod)
{
	uint8_t testData[4] = {0};
	ConversionTools::uint32To4Uint8Buffer(OUTPUT_COMPLETE_NOTIFICATION_ID, testData);

	ApplicationCallbackMock cbMock;
	CanControllerMock canCtrlMock;
	DisplayControllerMock displayCtrlMock;
	TouchControllerMock touchCtrlMock;
	LoggerMock loggerMock;

	auto appInstance = Application::GetInstance();
	appInstance->registerCallback(dynamic_cast<IApplicationCallback*>(&cbMock));
	appInstance->init(&canCtrlMock, &displayCtrlMock, &touchCtrlMock, &loggerMock);

	EXPECT_CALL(cbMock, enqueueDisplayInstruction(_)).Times(1);

	appInstance->processCanTask((void*)testData);

	// tear down
	appInstance->CleanUp();
}


