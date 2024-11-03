#include "gtest/gtest.h"
#include "TimerInUseManager.hpp"


TEST(TimerInUseManager_Tests, PassingTest)
{
	uint32_t tNumber = TimerInUseManager::reserveFreeTimerNumber();

	EXPECT_TRUE(tNumber != NO_TIMER_AVAILABLE);
	EXPECT_EQ(tNumber, 1);

	// tear down
	TimerInUseManager::releaseReservedTimerNumber(tNumber);
}

TEST(TimerInUseManager_Tests, ReleaseTest)
{
	uint32_t firstNumber = TimerInUseManager::reserveFreeTimerNumber();
	TimerInUseManager::releaseReservedTimerNumber(firstNumber);
	uint32_t secondNumber = TimerInUseManager::reserveFreeTimerNumber();

	EXPECT_EQ(firstNumber, 1);
	EXPECT_EQ(secondNumber, 1);

	// tear down
	TimerInUseManager::releaseReservedTimerNumber(secondNumber);
}

TEST(TimerInUseManager_Tests, TimerExceededTest)
{
	uint32_t tNumber;

	for(uint32_t i = 0; i < 14; i++)
	{
		tNumber = TimerInUseManager::reserveFreeTimerNumber();
	}

	EXPECT_TRUE(tNumber == NO_TIMER_AVAILABLE);

	// tear down
	for(uint32_t i = 0; i < 14; i++)
	{
		TimerInUseManager::releaseReservedTimerNumber(i);
	}
}

TEST(TimerInUseManager_Tests, NumberOrderTest)
{
	uint32_t tNumber;

	for(uint32_t i = 0; i < 6; i++)
	{
		tNumber = TimerInUseManager::reserveFreeTimerNumber();
	}

	EXPECT_EQ(tNumber, 7);

	// tear down
	for(uint32_t i = 0; i < 8; i++)
	{
		TimerInUseManager::releaseReservedTimerNumber(i);
	}
}


