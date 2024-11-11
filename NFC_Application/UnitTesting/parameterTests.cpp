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

TEST(ParameterTests, TestStringParameter)
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

TEST(ParameterTests, TestStructParameter)
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

