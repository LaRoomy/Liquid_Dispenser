#include "gtest/gtest.h"
#include "Application.hpp"

int main(int ac, char* av[])
{
  testing::InitGoogleTest(&ac, av);
  return RUN_ALL_TESTS();
}

TEST(BasicTest, IsNotNullTest)
{
	auto app = Application::GetInstance();
	EXPECT_TRUE(app != nullptr);
}


