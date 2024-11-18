#include "gtest/gtest.h"
#include "mocks.hpp"
#include "Application.hpp"
#include <ConversionTool.hpp>
#include <can_communication_codes.h>

using ::testing::_;
using ::testing::Eq;
using ::testing::DoAll;
using ::testing::SaveArg;

TEST(CanParameterTests, CanReceivedData_OperationInitiatedDataEnqueued)
{
	void * ptr = nullptr;
	uint8_t testData[] = {0,0,1,244}; // 500ml
	uint8_t expectedData[] = {0,0,0,OUTPUT_OPERATION_INITIATED_NOTIFICATION_ID,4,0,0,1,244};

	ApplicationCallbackMock cbMock;
	CanControllerMock canCtrlMock;
	FlowControllerMock flowCtrlMock;
	LoggerMock loggerMock;

	CAN_DATA cData;
	cData.dataSize = 4U;
	cData.data = testData;
	cData.stdID = OUTPUT_OPERATION_INITIATED_NOTIFICATION_ID;

	auto appInstance = Application::GetInstance();
	appInstance->registerCallback(dynamic_cast<IApplicationCallback*>(&cbMock));
	appInstance->init(&canCtrlMock, &flowCtrlMock, &loggerMock);

	ON_CALL(cbMock, enqueueCanData(_))
		.WillByDefault(DoAll(SaveArg<0>(&ptr)));

	canCtrlMock.triggerCanCallbackDataReceived(&cData);

	uint8_t * enqueuedData = *((uint8_t**)ptr);

	ASSERT_EQ(enqueuedData[0], expectedData[0]);
	ASSERT_EQ(enqueuedData[1], expectedData[1]);
	ASSERT_EQ(enqueuedData[2], expectedData[2]);
	ASSERT_EQ(enqueuedData[3], expectedData[3]);
	ASSERT_EQ(enqueuedData[4], expectedData[4]);
	ASSERT_EQ(enqueuedData[5], expectedData[5]);
	ASSERT_EQ(enqueuedData[6], expectedData[6]);
	ASSERT_EQ(enqueuedData[7], expectedData[7]);
	ASSERT_EQ(enqueuedData[8], expectedData[8]);

	// tear down
	appInstance->cleanUp();
}

TEST(CanParameterTests, CanReceivedData_OutputConfirmedDataEnqueued)
{
	void * ptr = nullptr;
	uint8_t testData[] = {0};
	uint8_t expectedData[] = {0,0,0,OUTPUT_CONFIRMED_NOTIFICATON_ID,1,0};

	ApplicationCallbackMock cbMock;
	CanControllerMock canCtrlMock;
	FlowControllerMock flowCtrlMock;
	LoggerMock loggerMock;

	CAN_DATA cData;
	cData.dataSize = 1U;
	cData.data = testData;
	cData.stdID = OUTPUT_CONFIRMED_NOTIFICATON_ID;

	auto appInstance = Application::GetInstance();
	appInstance->registerCallback(dynamic_cast<IApplicationCallback*>(&cbMock));
	appInstance->init(&canCtrlMock, &flowCtrlMock, &loggerMock);

	ON_CALL(cbMock, enqueueCanData(_))
		.WillByDefault(DoAll(SaveArg<0>(&ptr)));

	canCtrlMock.triggerCanCallbackDataReceived(&cData);

	uint8_t * enqueuedData = *((uint8_t**)ptr);

	// assert
	ASSERT_EQ(enqueuedData[0], expectedData[0]);
	ASSERT_EQ(enqueuedData[1], expectedData[1]);
	ASSERT_EQ(enqueuedData[2], expectedData[2]);
	ASSERT_EQ(enqueuedData[3], expectedData[3]);
	ASSERT_EQ(enqueuedData[4], expectedData[4]);
	ASSERT_EQ(enqueuedData[5], expectedData[5]);

	// tear down
	appInstance->cleanUp();
}


TEST(CanParameterTests, CanReceivedData_SystemErrorDataEnqueued)
{
	void * ptr = nullptr;
	const char errMsg[] = "Test Message";

	uint8_t testData[12];
	uint8_t expectedData[] = {0,0,0,SYSTEM_ERROR_OCCURRED_NOTIFICATION_ID, 12U, 0x54, 0x65, 0x73, 0x74, 0x20, 0x4D, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65};

	auto len = ConversionTools::convertCharArrayToUint8Array(errMsg, testData);
	len += 5;

	ApplicationCallbackMock cbMock;
	CanControllerMock canCtrlMock;
	FlowControllerMock flowCtrlMock;
	LoggerMock loggerMock;

	CAN_DATA cData;
	cData.dataSize = 12U;
	cData.data = testData;
	cData.stdID = SYSTEM_ERROR_OCCURRED_NOTIFICATION_ID;

	auto appInstance = Application::GetInstance();
	appInstance->registerCallback(dynamic_cast<IApplicationCallback*>(&cbMock));
	appInstance->init(&canCtrlMock, &flowCtrlMock, &loggerMock);

	ON_CALL(cbMock, enqueueCanData(_))
		.WillByDefault(DoAll(SaveArg<0>(&ptr)));

	canCtrlMock.triggerCanCallbackDataReceived(&cData);

	uint8_t * enqueuedData = *((uint8_t**)ptr);

	// assert
	for(uint32_t i = 0; i < (uint32_t)len; i++)
	{
		ASSERT_EQ(enqueuedData[i], expectedData[i]);
	}

	// tear down
	appInstance->cleanUp();
}
