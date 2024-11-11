#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ConversionTool.hpp"


TEST(ConversionToolTests, uint32To4Uint8Buffer_CorrectConversion)
{
	uint32_t testValue = 0xa1b2c3d4;
	uint8_t buffer[4];

	ConversionTools::uint32To4Uint8Buffer(testValue, buffer);

	EXPECT_EQ(buffer[0], 0xa1);
	EXPECT_EQ(buffer[1], 0xb2);
	EXPECT_EQ(buffer[2], 0xc3);
	EXPECT_EQ(buffer[3], 0xd4);
}

TEST(ConversionToolTests, FourUint8ToUint32_CorrectConversion)
{
	uint32_t expectedValue = 0xa1b2c3d4;
	uint8_t buffer[] = { 0xa1, 0xb2, 0xc3, 0xd4 };

	uint32_t conversionResult = ConversionTools::fourUint8ToUint32(buffer[0], buffer[1], buffer[2], buffer[3]);

	EXPECT_EQ(conversionResult, expectedValue);
}

TEST(ConversionToolTests, ExtractOutputAmountReturnsCorrectAmountFromString)
{
	char testString[] = "OUTPUT:200";

	auto value = ConversionTools::extractOutputAmount(testString);

	EXPECT_EQ(value, 200);
}

TEST(ConversionToolTests, ExtractOutputAmountReturnsCorrectAmountFromString_LargeNumber)
{
	char testString[] = "OUTPUT:123456";

	auto value = ConversionTools::extractOutputAmount(testString);

	EXPECT_EQ(value, 123456);
}

TEST(ConversionToolTests, ExtractOutputAmountReturnsInvalidAmountDueToIncorrectSequenceFormat)
{
	char testString[] = "OUTUT:200";

	auto value = ConversionTools::extractOutputAmount(testString);

	EXPECT_EQ(value, INVALID_OUTPUT_AMOUNT);
}

TEST(ConversionToolTests, ExtractOutputAmountReturnsInvalidAmountDueToIncorrectNumberFormat)
{
	char testString[] = "OUTUT:2a0";

	auto value = ConversionTools::extractOutputAmount(testString);

	EXPECT_EQ(value, INVALID_OUTPUT_AMOUNT);
}

