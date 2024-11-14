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

TEST(TouchActionTests, TouchControllerGetCalledAtAll)
{
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

	appInstance->processCanTask((void*)testData);// set app instance to confirm-cancel screen
	ASSERT_EQ(Application::CONFIRM_OR_CANCEL_STATE, appInstance->getCurrentState());

	EXPECT_CALL(touchCtrlMock, isTouchPressed())
		.Times(1)
		.WillOnce(Return(true));

	EXPECT_CALL(touchCtrlMock, getCoordinates(_,_))
		.Times(1);

	appInstance->processTouchTask();

	// tear down
	appInstance->CleanUp();
}

TEST(TouchActionTests, ConfirmButtonPressed)
{
	uint8_t testData[4] = {0};
	ConversionTools::uint32To4Uint8Buffer(OUTPUT_OPERATION_INITIATED_NOTIFICATION_ID, testData);

	uint8_t rawData[] = {0};

	CAN_DATA outData;
	outData.stdID = OUTPUT_CONFIRMED_NOTIFICATON_ID;
	outData.dataSize = 1;
	outData.data = rawData;

	ApplicationCallbackMock cbMock;
	CanControllerMock canCtrlMock;
	DisplayControllerMock displayCtrlMock;
	TouchControllerMock touchCtrlMock;
	LoggerMock loggerMock;

	auto appInstance = Application::GetInstance();
	appInstance->registerCallback(dynamic_cast<IApplicationCallback*>(&cbMock));
	appInstance->init(&canCtrlMock, &displayCtrlMock, &touchCtrlMock, &loggerMock);

	// force app instance to confirm-cancel screen
	appInstance->processCanTask((void*)testData);
	// confirm state
	ASSERT_EQ(Application::CONFIRM_OR_CANCEL_STATE, appInstance->getCurrentState());

	// force 'isTouchPressed' to return true when called
	ON_CALL(touchCtrlMock, isTouchPressed()).WillByDefault(Return(true));

	// set output coordinate parameter when called
	ON_CALL(touchCtrlMock, getCoordinates(_,_)).WillByDefault(DoAll(SetArgPointee<0>(80), SetArgPointee<1>(120), Return(true)));

	// pre-assert: check if correct data was written to can controller
	EXPECT_CALL(canCtrlMock, writeData(Pointee(Eq(outData)))).Times(1);

	// act
	appInstance->processTouchTask();

	// tear down
	appInstance->CleanUp();
}

TEST(TouchActionTests, CancelButtonPressed)
{
	uint8_t testData[4] = {0};
	ConversionTools::uint32To4Uint8Buffer(OUTPUT_OPERATION_INITIATED_NOTIFICATION_ID, testData);

	uint8_t rawData[] = {0};

	CAN_DATA outData;
	outData.stdID = OUTPUT_CANCELLED_NOTIFICATION_ID;
	outData.dataSize = 1;
	outData.data = rawData;

	ApplicationCallbackMock cbMock;
	CanControllerMock canCtrlMock;
	DisplayControllerMock displayCtrlMock;
	TouchControllerMock touchCtrlMock;
	LoggerMock loggerMock;

	auto appInstance = Application::GetInstance();
	appInstance->registerCallback(dynamic_cast<IApplicationCallback*>(&cbMock));
	appInstance->init(&canCtrlMock, &displayCtrlMock, &touchCtrlMock, &loggerMock);

	// force app instance to confirm-cancel screen
	appInstance->processCanTask((void*)testData);
	// confirm state
	ASSERT_EQ(Application::CONFIRM_OR_CANCEL_STATE, appInstance->getCurrentState());

	// force 'isTouchPressed' to return true when called
	ON_CALL(touchCtrlMock, isTouchPressed()).WillByDefault(Return(true));

	// set output coordinate parameter when called
	ON_CALL(touchCtrlMock, getCoordinates(_,_)).WillByDefault(DoAll(SetArgPointee<0>(240), SetArgPointee<1>(120), Return(true)));

	// pre-assert: check if correct data was written to can controller
	EXPECT_CALL(canCtrlMock, writeData(Pointee(Eq(outData)))).Times(1);

	// act
	appInstance->processTouchTask();

	// tear down
	appInstance->CleanUp();
}

TEST(TouchActionTests, TouchedButNoButtonPressed)
{
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

	// force app instance to confirm-cancel screen
	appInstance->processCanTask((void*)testData);
	// confirm state
	ASSERT_EQ(Application::CONFIRM_OR_CANCEL_STATE, appInstance->getCurrentState());

	// force 'isTouchPressed' to return true when called
	ON_CALL(touchCtrlMock, isTouchPressed()).WillByDefault(Return(true));

	// set output coordinate parameter when called
	ON_CALL(touchCtrlMock, getCoordinates(_,_)).WillByDefault(DoAll(SetArgPointee<0>(20), SetArgPointee<1>(20), Return(true)));

	// pre-assert: the method should not be called (0 times)
	EXPECT_CALL(canCtrlMock, writeData(_)).Times(0);

	// act
	appInstance->processTouchTask();

	// tear down
	appInstance->CleanUp();
}



