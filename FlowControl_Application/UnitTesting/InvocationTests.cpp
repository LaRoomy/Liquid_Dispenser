#include "gtest/gtest.h"
#include "mocks.hpp"
#include "Application.hpp"
#include <ConversionTool.hpp>
#include <can_communication_codes.h>

using ::testing::_;
using ::testing::Eq;

// TODO: ConversionTools -> proceed and Test !!
// TODO: can enqueue parameter tests -> proceed

TEST(InvocationTests, WrongParameterInvokesCriticalErrorMethod)
{
	ApplicationCallbackMock cbMock;

	auto appInstance = Application::GetInstance();
	appInstance->registerCallback(dynamic_cast<IApplicationCallback*>(&cbMock));

	EXPECT_CALL(cbMock, handleCriticalError(_)).Times(1);

	appInstance->init(nullptr, nullptr, nullptr);
}

TEST(InvocationTests, CanReceivedDataAndInvokesEnqueueMethod)
{
	uint8_t testData[] = {1,2,3,4,9,8,7};

	ApplicationCallbackMock cbMock;
	CanControllerMock canCtrlMock;
	FlowControllerMock flowCtrlMock;
	LoggerMock loggerMock;

	CAN_DATA cData;
	cData.dataSize = 7U;
	cData.data = testData;
	cData.stdID = 0x01U;

	auto appInstance = Application::GetInstance();
	appInstance->registerCallback(dynamic_cast<IApplicationCallback*>(&cbMock));
	appInstance->init(&canCtrlMock, &flowCtrlMock, &loggerMock);

	EXPECT_CALL(cbMock, enqueueCanData(_)).Times(1);

	// act
	canCtrlMock.triggerCanCallbackDataReceived(&cData);

	// tear down
	appInstance->cleanUp();
}

TEST(InvocationTests, CanReceivedStartFlowOutputMessageAndInvokesFlowStartOperation)
{
	uint8_t data[] = {0,0,0,OUTPUT_CONFIRMED_NOTIFICATON_ID};

	ApplicationCallbackMock cbMock;
	CanControllerMock canCtrlMock;
	FlowControllerMock flowCtrlMock;
	LoggerMock loggerMock;

	auto appInstance = Application::GetInstance();
	appInstance->registerCallback(dynamic_cast<IApplicationCallback*>(&cbMock));
	appInstance->init(&canCtrlMock, &flowCtrlMock, &loggerMock);

	EXPECT_CALL(cbMock, startFlowOperationTask()).Times(1);

	// act
	appInstance->processCanTask(data);

	// tear down
	appInstance->cleanUp();
}

TEST(InvocationTests, CanReceivedStopFlowOutputMessageAndInvokesFlowStopMethod)
{
	uint8_t data[] = {0,0,0,OUTPUT_STOPPED_NOTIFICATION_ID};

	ApplicationCallbackMock cbMock;
	CanControllerMock canCtrlMock;
	FlowControllerMock flowCtrlMock;
	LoggerMock loggerMock;

	auto appInstance = Application::GetInstance();
	appInstance->registerCallback(dynamic_cast<IApplicationCallback*>(&cbMock));
	appInstance->init(&canCtrlMock, &flowCtrlMock, &loggerMock);

	EXPECT_CALL(flowCtrlMock, stopWaterFlow()).Times(1);

	// act
	appInstance->processCanTask(data);

	// tear down
	appInstance->cleanUp();
}

TEST(InvocationTests, StartWaterFlowMethodInvoked)
{
	uint8_t data[] = {0,0,0,OUTPUT_OPERATION_INITIATED_NOTIFICATION_ID,4,0,0,0,200};

	ApplicationCallbackMock cbMock;
	CanControllerMock canCtrlMock;
	FlowControllerMock flowCtrlMock;
	LoggerMock loggerMock;

	auto appInstance = Application::GetInstance();
	appInstance->registerCallback(dynamic_cast<IApplicationCallback*>(&cbMock));
	appInstance->init(&canCtrlMock, &flowCtrlMock, &loggerMock);
	appInstance->processCanTask(data);

	EXPECT_CALL(flowCtrlMock, startWaterFlow(_)).Times(1);

	// act
	appInstance->processFlowControlOperation();

	// tear down
	appInstance->cleanUp();
}



