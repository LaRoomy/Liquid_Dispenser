#include "gtest/gtest.h"
#include "SerialInUseManager.hpp"

int main(int ac, char* av[])
{
  testing::InitGoogleTest(&ac, av);
  return RUN_ALL_TESTS();
}

TEST(SerialInUseManagerTests, HappyPath)
{
	bool result =
			SerialInUseManager::reserveFreeUartNumber(2, nullptr);

	ASSERT_TRUE(result);

	// tear down
	SerialInUseManager::releaseReservedUartNumber(2);
}

TEST(SerialInUseManagerTests, ReleaseTest)
{
	SerialInUseManager::reserveFreeUartNumber(2, nullptr);
	SerialInUseManager::releaseReservedUartNumber(2);
	ASSERT_TRUE(SerialInUseManager::isUartAvailable(2));
}

TEST(SerialInUseManagerTests, NotAvailableTest)
{
	SerialInUseManager::reserveFreeUartNumber(2, nullptr);
	ASSERT_FALSE(SerialInUseManager::isUartAvailable(2));

	// tear down
	SerialInUseManager::releaseReservedUartNumber(2);
}

TEST(SerialInUseManagerTests, UseAllUartsTest)
{
	bool result = true;

	for(uint32_t i = 1; i < 7; i++)
	{
		if(SerialInUseManager::reserveFreeUartNumber(i, nullptr) == false)
		{
			result = false;
		}
	}

	ASSERT_TRUE(result);

	// tear down
	for(uint32_t i = 1; i < 7; i++)
	{
		SerialInUseManager::releaseReservedUartNumber(i);
	}
}

TEST(SerialInUseManagerTests, StoreReferenceForUartTest)
{
	bool result = true;
	int refValues[6] = { 1, 2, 3, 4, 5, 6 };

	for(uint32_t i = 1; i < 7; i++)
	{
		SerialInUseManager::reserveFreeUartNumber(i, &refValues[i - 1]);
	}

	for(uint32_t i = 1; i < 7; i++)
	{
		auto ref =
				reinterpret_cast<int*>(SerialInUseManager::getReferenceForUartNumber(i));

		if(ref != &refValues[i - 1] || *ref != refValues[i - 1] )
		{
			result = false;
		}
	}

	ASSERT_TRUE(result);

	// tear down
	for(uint32_t i = 1; i < 7; i++)
	{
		SerialInUseManager::releaseReservedUartNumber(i);
	}
}




