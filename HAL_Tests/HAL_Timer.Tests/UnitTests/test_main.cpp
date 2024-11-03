#include "gtest/gtest.h"

int main(int ac, char* av[])
{
  testing::InitGoogleTest(&ac, av);
  return RUN_ALL_TESTS();
}

TEST(BasicTest, TestSuiteTest)
{
	uint32_t testValue = 22;
	EXPECT_EQ(22, testValue);
}
