#include "gtest/gtest.h"
#include "mocks.hpp"
#include "Application.hpp"
#include <ConversionTool.hpp>
#include <can_communication_codes.h>

using ::testing::_;
using ::testing::Eq;
using ::testing::Pointee;
using ::testing::Invoke;
using ::testing::DoAll;
using ::testing::SaveArg;

TEST(ParameterTests, ReceivedOutputCompleteAndEnqueuesRespectiveDisplayInstruction)
{
	uint8_t testData[4] = {0};
	ConversionTools::uint32To4Uint8Buffer(OUTPUT_COMPLETE_NOTIFICATION_ID, testData);

	void * pInstruction = nullptr;

	ApplicationCallbackMock cbMock;
	CanControllerMock canCtrlMock;
	DisplayControllerMock displayCtrlMock;
	TouchControllerMock touchCtrlMock;
	LoggerMock loggerMock;

	auto appInstance = Application::GetInstance();
	appInstance->registerCallback(dynamic_cast<IApplicationCallback*>(&cbMock));
	appInstance->init(&canCtrlMock, &displayCtrlMock, &touchCtrlMock, &loggerMock);

	EXPECT_CALL(cbMock, enqueueDisplayInstruction(_))
		.Times(1)
		.WillOnce(DoAll(SaveArg<0>(&pInstruction)));

	appInstance->processCanTask((void*)testData);

	uint32_t instruct = *(*((uint32_t**)pInstruction));

	ASSERT_EQ(DISPLAYINSTRUCTION_SHOW_INACTIVE_SCREEN, instruct);

	// tear down
	appInstance->CleanUp();
}

TEST(ParameterTests, ReceivedOperationInitiatedAndEnqueuesRespectiveDisplayInstruction)
{
	uint8_t testData[4] = {0};
	ConversionTools::uint32To4Uint8Buffer(OUTPUT_OPERATION_INITIATED_NOTIFICATION_ID, testData);

	void * pInstruction = nullptr;

	ApplicationCallbackMock cbMock;
	CanControllerMock canCtrlMock;
	DisplayControllerMock displayCtrlMock;
	TouchControllerMock touchCtrlMock;
	LoggerMock loggerMock;

	auto appInstance = Application::GetInstance();
	appInstance->registerCallback(dynamic_cast<IApplicationCallback*>(&cbMock));
	appInstance->init(&canCtrlMock, &displayCtrlMock, &touchCtrlMock, &loggerMock);

	EXPECT_CALL(cbMock, enqueueDisplayInstruction(_))
		.Times(1)
		.WillOnce(DoAll(SaveArg<0>(&pInstruction)));

	appInstance->processCanTask((void*)testData);

	uint32_t instruct = *(*((uint32_t**)pInstruction));

	ASSERT_EQ(DISPLAYINSTRUCTION_SHOW_CONFIRM_CANCEL_SCREEN, instruct);

	// tear down
	appInstance->CleanUp();
}

TEST(ParameterTests, ReceivedErrorAndEnqueuesRespectiveDisplayInstruction)
{
	uint8_t testData[4] = {0};
	ConversionTools::uint32To4Uint8Buffer(SYSTEM_ERROR_OCCURRED_NOTIFICATION_ID, testData);

	void * pInstruction = nullptr;

	ApplicationCallbackMock cbMock;
	CanControllerMock canCtrlMock;
	DisplayControllerMock displayCtrlMock;
	TouchControllerMock touchCtrlMock;
	LoggerMock loggerMock;

	auto appInstance = Application::GetInstance();
	appInstance->registerCallback(dynamic_cast<IApplicationCallback*>(&cbMock));
	appInstance->init(&canCtrlMock, &displayCtrlMock, &touchCtrlMock, &loggerMock);

	EXPECT_CALL(cbMock, enqueueDisplayInstruction(_))
		.Times(1)
		.WillOnce(DoAll(SaveArg<0>(&pInstruction)));

	appInstance->processCanTask((void*)testData);

	uint32_t instruct = *(*((uint32_t**)pInstruction));

	ASSERT_EQ(DISPLAYINSTRUCTION_SHOW_ERROR_SCREEN, instruct);

	// tear down
	appInstance->CleanUp();
}
