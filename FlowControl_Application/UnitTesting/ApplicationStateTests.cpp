#include "gtest/gtest.h"
#include "mocks.hpp"
#include "Application.hpp"
#include <ConversionTool.hpp>
#include <can_communication_codes.h>

TEST(ApplicationStateTests, ApplicationEntersActiveStateDueToInitiateCanNotification)
{
	uint8_t data[] = {0,0,0,OUTPUT_OPERATION_INITIATED_NOTIFICATION_ID,0,0,0,200};

	ApplicationCallbackMock cbMock;
	CanControllerMock canCtrlMock;
	FlowControllerMock flowCtrlMock;
	LoggerMock loggerMock;

	auto appInstance = Application::GetInstance();
	appInstance->registerCallback(dynamic_cast<IApplicationCallback*>(&cbMock));
	appInstance->init(&canCtrlMock, &flowCtrlMock, &loggerMock);

	// act
	appInstance->processCanTask(data);

	// assert
	ASSERT_EQ(appInstance->getApplicationState(), Application::ACTIVE_STATE);

	// tear down
	appInstance->cleanUp();
}

TEST(ApplicationStateTests, ApplicationEntersOutputStateDueToOutputConfirmedCanNotification)
{
	uint8_t confirm_data[] = {0,0,0,OUTPUT_CONFIRMED_NOTIFICATON_ID};
	uint8_t initiate_data[] = {0,0,0,OUTPUT_OPERATION_INITIATED_NOTIFICATION_ID,0,0,0,200};

	ApplicationCallbackMock cbMock;
	CanControllerMock canCtrlMock;
	FlowControllerMock flowCtrlMock;
	LoggerMock loggerMock;

	auto appInstance = Application::GetInstance();
	appInstance->registerCallback(dynamic_cast<IApplicationCallback*>(&cbMock));
	appInstance->init(&canCtrlMock, &flowCtrlMock, &loggerMock);

	// act
	appInstance->processCanTask(initiate_data);// goto active state
	appInstance->processCanTask(confirm_data);// confirm and goto output state
	appInstance->processFlowControlOperation();// start output

	// assert
	ASSERT_EQ(appInstance->getApplicationState(), Application::OUTPUT_STATE);

	// tear down
	appInstance->cleanUp();
}

TEST(ApplicationStateTests, ApplicationEntersInactiveStateDueToOutputCancelledCanNotification)
{
	uint8_t cancel_data[] = {0,0,0,OUTPUT_CANCELLED_NOTIFICATION_ID};
	uint8_t initiate_data[] = {0,0,0,OUTPUT_OPERATION_INITIATED_NOTIFICATION_ID,0,0,0,200};

	ApplicationCallbackMock cbMock;
	CanControllerMock canCtrlMock;
	FlowControllerMock flowCtrlMock;
	LoggerMock loggerMock;

	auto appInstance = Application::GetInstance();
	appInstance->registerCallback(dynamic_cast<IApplicationCallback*>(&cbMock));
	appInstance->init(&canCtrlMock, &flowCtrlMock, &loggerMock);

	// act
	appInstance->processCanTask(initiate_data);// goto active state
	appInstance->processCanTask(cancel_data);// cancel operation

	// assert
	ASSERT_EQ(appInstance->getApplicationState(), Application::INACTIVE_STATE);

	// tear down
	appInstance->cleanUp();
}





