#include "gtest/gtest.h"
#include "mocks.hpp"
#include "Application.hpp"
#include <ConversionTool.hpp>
#include <can_communication_codes.h>

using ::testing::_;
using ::testing::Eq;
using ::testing::DoAll;
using ::testing::SaveArg;

TEST(FlowControlAmountConversionTests, Amount200ReceivedAndConvertedTo_2dl)
{
	uint8_t data[] = {0,0,0,OUTPUT_OPERATION_INITIATED_NOTIFICATION_ID,4,0,0,0,200};
	uint32_t amount = 0;

	ApplicationCallbackMock cbMock;
	CanControllerMock canCtrlMock;
	FlowControllerMock flowCtrlMock;
	LoggerMock loggerMock;

	auto appInstance = Application::GetInstance();
	appInstance->registerCallback(dynamic_cast<IApplicationCallback*>(&cbMock));
	appInstance->init(&canCtrlMock, &flowCtrlMock, &loggerMock);
	appInstance->processCanTask(data);

	ON_CALL(flowCtrlMock, startWaterFlow(_))
		.WillByDefault(DoAll(SaveArg<0>(&amount)));

	// act
	appInstance->processFlowControlOperation();

	// assert
	ASSERT_EQ(amount, 2U); // expect 2 dl amount

	// tear down
	appInstance->cleanUp();
}

TEST(FlowControlAmountConversionTests, Amount200ReceivedAndConvertedTo_5dl)
{
	uint8_t data[] = {0,0,0,OUTPUT_OPERATION_INITIATED_NOTIFICATION_ID,4,0,0,1,244};
	uint32_t amount = 0;

	ApplicationCallbackMock cbMock;
	CanControllerMock canCtrlMock;
	FlowControllerMock flowCtrlMock;
	LoggerMock loggerMock;

	auto appInstance = Application::GetInstance();
	appInstance->registerCallback(dynamic_cast<IApplicationCallback*>(&cbMock));
	appInstance->init(&canCtrlMock, &flowCtrlMock, &loggerMock);
	appInstance->processCanTask(data);

	ON_CALL(flowCtrlMock, startWaterFlow(_))
		.WillByDefault(DoAll(SaveArg<0>(&amount)));

	// act
	appInstance->processFlowControlOperation();

	// assert
	ASSERT_EQ(amount, 5U); // expect 5 dl amount

	// tear down
	appInstance->cleanUp();
}

TEST(FlowControlAmountConversionTests, Amount033ReceivedAndKept)
{
	uint8_t data[] = {0,0,0,OUTPUT_OPERATION_INITIATED_NOTIFICATION_ID,4,0xff,0xff,0xff,2};
	uint32_t amount = 0;

	ApplicationCallbackMock cbMock;
	CanControllerMock canCtrlMock;
	FlowControllerMock flowCtrlMock;
	LoggerMock loggerMock;

	auto appInstance = Application::GetInstance();
	appInstance->registerCallback(dynamic_cast<IApplicationCallback*>(&cbMock));
	appInstance->init(&canCtrlMock, &flowCtrlMock, &loggerMock);
	appInstance->processCanTask(data);

	ON_CALL(flowCtrlMock, startWaterFlow(_))
		.WillByDefault(DoAll(SaveArg<0>(&amount)));

	// act
	appInstance->processFlowControlOperation();

	// assert
	ASSERT_EQ(amount, FLOWCONTROL_QUANTITY_0_33_L); // 0.33 l amount expected

	// tear down
	appInstance->cleanUp();
}

TEST(FlowControlAmountConversionTests, AmountIndeterminateReceivedAndKept)
{
	uint8_t data[] = {0,0,0,OUTPUT_OPERATION_INITIATED_NOTIFICATION_ID,4,0xff,0xff,0xff,0};
	uint32_t amount = 0;

	ApplicationCallbackMock cbMock;
	CanControllerMock canCtrlMock;
	FlowControllerMock flowCtrlMock;
	LoggerMock loggerMock;

	auto appInstance = Application::GetInstance();
	appInstance->registerCallback(dynamic_cast<IApplicationCallback*>(&cbMock));
	appInstance->init(&canCtrlMock, &flowCtrlMock, &loggerMock);
	appInstance->processCanTask(data);

	ON_CALL(flowCtrlMock, startWaterFlow(_))
		.WillByDefault(DoAll(SaveArg<0>(&amount)));

	// act
	appInstance->processFlowControlOperation();

	// assert
	ASSERT_EQ(amount, FLOWCONTROL_QUANTITY_INDETERMINATE); // indeterminate amount expected

	// tear down
	appInstance->cleanUp();
}
