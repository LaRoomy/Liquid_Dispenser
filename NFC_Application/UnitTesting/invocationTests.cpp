#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Application.hpp"
#include "mocks.hpp"

using ::testing::_;

TEST(InvocationTests, InvokesCriticalErrorCallbackMethod)
{
	ApplicationCallbackMock cbMock;

	auto app = Application::GetInstance();
	app->registerCallback(dynamic_cast<IApplicationCallback*>(&cbMock));

	EXPECT_CALL(cbMock, onCriticalError(_)).Times(1);

	app->init(nullptr, nullptr, nullptr);
}

TEST(InvocationTests, InvokesCanEnqueueCallbackMethod)
{
	uint8_t testData[] = {1,2,3,4,9,8,7};

	ApplicationCallbackMock cbMock;
	CAN_DATA cData;
	cData.dataSize = 7U;
	cData.data = testData;
	cData.stdID = 0x01U;

	auto app = Application::GetInstance();
	app->registerCallback(dynamic_cast<IApplicationCallback*>(&cbMock));

	EXPECT_CALL(cbMock, enqueueCanData(_)).Times(1);

	app->canDataReceived(&cData);
}

TEST(InvocationTests, InvokesNfcEnqueueCallbackMethod)
{
	const char testString[] = "hello test";
	ApplicationCallbackMock cbMock;

	auto app = Application::GetInstance();
	app->registerCallback(dynamic_cast<IApplicationCallback*>(&cbMock));

	EXPECT_CALL(cbMock, enqueueNfcData(_)).Times(1);

	app->nfcDataReceived(testString);
}
