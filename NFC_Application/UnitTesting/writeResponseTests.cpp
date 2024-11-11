#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Application.hpp"
#include "mocks.hpp"
#include "can_communication_codes.h"
#include "ConversionTool.hpp"

using ::testing::_;
using ::testing::StrEq;
using ::testing::Eq;
using ::testing::Pointee;

TEST(WriteResponseTests, NfcReceivedDataAndWritesCommandToCanController)
{
	char testData[] = "OUTPUT:300";

	uint8_t data[4];
	ConversionTools::uint32To4Uint8Buffer(300, data);

	CAN_DATA canData;
	canData.stdID = OUTPUT_OPERATION_INITIATED_NOTIFICATION_ID;
	canData.dataSize = 4;
	canData.data = data;

	CanInterfaceMock canInterfaceMock;
	NfcInterfaceMock nfcInterfaceMock;
	LoggerMock loggerMock;

	auto app = Application::GetInstance();
	app->init(&nfcInterfaceMock, &canInterfaceMock, &loggerMock);

	EXPECT_CALL(canInterfaceMock, writeData(Pointee(Eq(canData)))).Times(1);

	app->processNfcTask((void*)testData);
}

TEST(WriteResponseTests, NfcReceivedDataAndWritesResponseToNfcController)
{
	char testData[] = "OUTPUT:550";

	CanInterfaceMock canInterfaceMock;
	NfcInterfaceMock nfcInterfaceMock;
	LoggerMock loggerMock;

	auto app = Application::GetInstance();
	app->init(&nfcInterfaceMock, &canInterfaceMock, &loggerMock);

	EXPECT_CALL(nfcInterfaceMock, writeData(StrEq("COMMAND:CONFIRMED"))).Times(1);

	app->processNfcTask((void*)testData);
}

TEST(WriteResponseTests, NfcReceivedDataAndEntersBusyState)
{
	char testData[] = "OUTPUT:300";

	CanInterfaceMock canInterfaceMock;
	NfcInterfaceMock nfcInterfaceMock;
	LoggerMock loggerMock;

	auto app = Application::GetInstance();
	app->init(&nfcInterfaceMock, &canInterfaceMock, &loggerMock);
	app->processNfcTask((void*)testData);

	EXPECT_EQ(app->getCurrentState(), Application::BUSY_STATE);
}

TEST(WriteResponseTests, CanReceivedDataAndInvokesEnqueueCallbackMethod)
{
	uint8_t testData[] = { 0, 1, 2 };
	CAN_DATA cData;
	cData.data = testData;
	cData.dataSize = 3;
	cData.stdID = OUTPUT_COMPLETE_NOTIFICATION_ID;

	ApplicationCallbackMock appCallbackMock;
	CanInterfaceMock canInterfaceMock;
	NfcInterfaceMock nfcInterfaceMock;
	LoggerMock loggerMock;

	auto app = Application::GetInstance();
	app->registerCallback(&appCallbackMock);
	app->init(&nfcInterfaceMock, &canInterfaceMock, &loggerMock);

	EXPECT_CALL(appCallbackMock, enqueueCanData(_)).Times(1);

	canInterfaceMock.triggerCanCallbackDataReceived(&cData);
}

TEST(WriteResponseTests, NfcReceivedDataAndInvokesEnqueueCallbackMethod)
{
	char testData[] = "hello test";

	ApplicationCallbackMock appCallbackMock;
	CanInterfaceMock canInterfaceMock;
	NfcInterfaceMock nfcInterfaceMock;
	LoggerMock loggerMock;

	auto app = Application::GetInstance();
	app->registerCallback(&appCallbackMock);
	app->init(&nfcInterfaceMock, &canInterfaceMock, &loggerMock);

	EXPECT_CALL(appCallbackMock, enqueueNfcData(_)).Times(1);

	nfcInterfaceMock.triggerNfcCallbackDataReceived(testData);
}

TEST(WriteResponseTests, CanRecievedOperationCompleteIDAndGoesIntoNormalState)
{
	uint8_t testData[8] = { 0 };
	ConversionTools::uint32To4Uint8Buffer(OUTPUT_COMPLETE_NOTIFICATION_ID, testData);

	CanInterfaceMock canInterfaceMock;
	NfcInterfaceMock nfcInterfaceMock;
	LoggerMock loggerMock;

	auto app = Application::GetInstance();
	app->init(&nfcInterfaceMock, &canInterfaceMock, &loggerMock);
	app->processCanTask((void*)testData);

	EXPECT_EQ(Application::NORMAL_STATE, app->getCurrentState());
}

TEST(WriteResponseTests, CanRecievedOutputCancelledIDAndGoesIntoNormalState)
{
	uint8_t testData[8] = { 0 };
	ConversionTools::uint32To4Uint8Buffer(OUTPUT_CANCELLED_NOTIFICATION_ID, testData);

	CanInterfaceMock canInterfaceMock;
	NfcInterfaceMock nfcInterfaceMock;
	LoggerMock loggerMock;

	auto app = Application::GetInstance();
	app->init(&nfcInterfaceMock, &canInterfaceMock, &loggerMock);
	app->processCanTask((void*)testData);

	EXPECT_EQ(Application::NORMAL_STATE, app->getCurrentState());
}

TEST(WriteResponseTests, CanRecievedOutputConfirmedIDAndGoesIntoBusyState)
{
	uint8_t testData[8] = { 0 };
	ConversionTools::uint32To4Uint8Buffer(OUTPUT_CONFIRMED_NOTIFICATON_ID, testData);

	CanInterfaceMock canInterfaceMock;
	NfcInterfaceMock nfcInterfaceMock;
	LoggerMock loggerMock;

	auto app = Application::GetInstance();
	app->init(&nfcInterfaceMock, &canInterfaceMock, &loggerMock);
	app->processCanTask((void*)testData);

	EXPECT_EQ(Application::BUSY_STATE, app->getCurrentState());
}

TEST(WriteResponseTests, CanRecievedSystemErrorIDAndGoesIntoErrorState)
{
	uint8_t testData[8] = { 0 };
	ConversionTools::uint32To4Uint8Buffer(SYSTEM_ERROR_OCCURRED_NOTIFICATION_ID, testData);

	CanInterfaceMock canInterfaceMock;
	NfcInterfaceMock nfcInterfaceMock;
	LoggerMock loggerMock;

	auto app = Application::GetInstance();
	app->init(&nfcInterfaceMock, &canInterfaceMock, &loggerMock);
	app->processCanTask((void*)testData);

	EXPECT_EQ(Application::ERROR_STATE, app->getCurrentState());
}
