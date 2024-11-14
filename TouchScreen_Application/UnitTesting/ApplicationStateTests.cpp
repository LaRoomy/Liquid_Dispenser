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
using ::testing::Return;
using ::testing::SetArgPointee;

TEST(ApplicationStateTests, EnterConfirmOrCancelStateDueToInitiateCanTransmission)
{
	// arrange
	uint8_t testData[4] = {0};
	ConversionTools::uint32To4Uint8Buffer(OUTPUT_OPERATION_INITIATED_NOTIFICATION_ID, testData);

	ApplicationCallbackMock cbMock;
	CanControllerMock canCtrlMock;
	DisplayControllerMock displayCtrlMock;
	TouchControllerMock touchCtrlMock;
	LoggerMock loggerMock;

	auto appInstance = Application::GetInstance();
	appInstance->registerCallback(dynamic_cast<IApplicationCallback*>(&cbMock));
	appInstance->init(&canCtrlMock, &displayCtrlMock, &touchCtrlMock, &loggerMock);

	// act
	appInstance->processCanTask((void*)testData);

	// assert
	ASSERT_EQ(Application::CONFIRM_OR_CANCEL_STATE, appInstance->getCurrentState());

	// tear down
	appInstance->CleanUp();
}

TEST(ApplicationStateTests, EnterInactiveStateDueToProcessCompleteCanTransmission)
{
	// arrange
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

	// act
	appInstance->processCanTask((void*)testData);

	// assert
	ASSERT_EQ(Application::INACTIVE_STATE, appInstance->getCurrentState());

	// tear down
	appInstance->CleanUp();
}

TEST(ApplicationStateTests, EnterErrorStateDueToErrorCanTransmission)
{
	// arrange
	uint8_t testData[8] = {0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41};
	ConversionTools::uint32To4Uint8Buffer(SYSTEM_ERROR_OCCURRED_NOTIFICATION_ID, testData);

	ApplicationCallbackMock cbMock;
	CanControllerMock canCtrlMock;
	DisplayControllerMock displayCtrlMock;
	TouchControllerMock touchCtrlMock;
	LoggerMock loggerMock;

	auto appInstance = Application::GetInstance();
	appInstance->registerCallback(dynamic_cast<IApplicationCallback*>(&cbMock));
	appInstance->init(&canCtrlMock, &displayCtrlMock, &touchCtrlMock, &loggerMock);

	// act
	appInstance->processCanTask((void*)testData);

	// assert
	ASSERT_EQ(Application::ERROR_STATE, appInstance->getCurrentState());

	// tear down
	appInstance->CleanUp();
}

TEST(ApplicationStateTests, EnterInactiveStateDueToCancelTouchEvent)
{
	// arrange
	uint8_t testData[8] = {0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41};
	ConversionTools::uint32To4Uint8Buffer(OUTPUT_OPERATION_INITIATED_NOTIFICATION_ID, testData);

	ApplicationCallbackMock cbMock;
	CanControllerMock canCtrlMock;
	DisplayControllerMock displayCtrlMock;
	TouchControllerMock touchCtrlMock;
	LoggerMock loggerMock;

	auto appInstance = Application::GetInstance();
	appInstance->registerCallback(dynamic_cast<IApplicationCallback*>(&cbMock));
	appInstance->init(&canCtrlMock, &displayCtrlMock, &touchCtrlMock, &loggerMock);
	// first enter confirm-or-cancel state
	appInstance->processCanTask((void*)testData);

	// force 'isTouchPressed' to return true when called
	ON_CALL(touchCtrlMock, isTouchPressed()).WillByDefault(Return(true));

	// set output coordinate parameter when called (cancel button position)
	ON_CALL(touchCtrlMock, getCoordinates(_,_)).WillByDefault(DoAll(SetArgPointee<0>(240), SetArgPointee<1>(120), Return(true)));

	// act
	appInstance->processTouchTask();

	// assert
	ASSERT_EQ(Application::INACTIVE_STATE, appInstance->getCurrentState());

	// tear down
	appInstance->CleanUp();
}

TEST(ApplicationStateTests, EnterStopOutputStateDueToConfirmTouchEvent)
{
	// arrange
	uint8_t testData[8] = {0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41};
	ConversionTools::uint32To4Uint8Buffer(OUTPUT_OPERATION_INITIATED_NOTIFICATION_ID, testData);

	ApplicationCallbackMock cbMock;
	CanControllerMock canCtrlMock;
	DisplayControllerMock displayCtrlMock;
	TouchControllerMock touchCtrlMock;
	LoggerMock loggerMock;

	auto appInstance = Application::GetInstance();
	appInstance->registerCallback(dynamic_cast<IApplicationCallback*>(&cbMock));
	appInstance->init(&canCtrlMock, &displayCtrlMock, &touchCtrlMock, &loggerMock);
	// first enter confirm-or-cancel state
	appInstance->processCanTask((void*)testData);

	// force 'isTouchPressed' to return true when called
	ON_CALL(touchCtrlMock, isTouchPressed()).WillByDefault(Return(true));

	// set output coordinate parameter when called (confirm button position)
	ON_CALL(touchCtrlMock, getCoordinates(_,_)).WillByDefault(DoAll(SetArgPointee<0>(80), SetArgPointee<1>(120), Return(true)));

	// act
	appInstance->processTouchTask();

	// assert
	ASSERT_EQ(Application::STOP_OUTPUT_STATE, appInstance->getCurrentState());

	// tear down
	appInstance->CleanUp();
}


