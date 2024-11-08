#ifndef CONVERSIONTOOL_HPP_
#define CONVERSIONTOOL_HPP_

#include <stdint.h>
#include <string.h>

constexpr uint32_t INVALID_OUTPUT_AMOUNT = 0xFFFFFF;

class ConversionTools
{
public:
	static int convertCharArrayToUint8Array(const char * cArr, uint8_t * uArr)
	{
		if(cArr != nullptr && uArr != nullptr)
		{
			auto len = strlen(cArr);
			if(len > 0)
			{
				for(uint32_t i = 0; i < len; i++)
				{
					if(cArr[i] == '\0')
						break;

					uArr[i] = (uint8_t)cArr[i];
				}
			}
			return (int)len;
		}
		return -1;
	}

	static void convertUint8ToCharArray(uint8_t * uArr, char * cArr, uint32_t size)
	{
		if(uArr != nullptr && cArr != nullptr)
		{
			if(size > 0)
			{
				for(uint32_t i = 0; i < size; i++)
				{
					cArr[i] = (char)uArr[i];
				}
				cArr[size] = '\0';
			}
		}
	}

	static void uint32To4Uint8Buffer(uint32_t value, uint8_t * buffer)
	{
		buffer[0] = (uint8_t)(value >> 24);
		buffer[1] = (uint8_t)((value >> 16) & 0x000000ff);
		buffer[2] = (uint8_t)((value >> 8) & 0x000000ff);
		buffer[3] = (uint8_t)(value & 0x000000ff);
	}

	static uint32_t fourUint8ToUint32(uint8_t ll, uint8_t lr, uint8_t rl, uint8_t rr)
	{
		return (((uint32_t)ll)<<24|((uint32_t)lr)<<16|((uint32_t)rl)<<8|((uint32_t)rr));
	}

	static uint32_t extractOutputAmount(const char * data)
	{
		if(data != nullptr)
		{
			int i = 0, j = 0;
			char sequence[10] = { '\0' };
			char outputAmount[10] = { '\0' };

			while(data[i] != ':')
			{
				if(data[i] == '\0')
				{
					return INVALID_OUTPUT_AMOUNT;
				}
				sequence[i] = data[i];
				i++;
			}
			i++;

			if(strcmp(sequence, "OUTPUT") == 0)
			{
				while(data[i] != '\0')
				{
					outputAmount[j] = data[i];
					i++; j++;
				}
				unsigned int value;
				auto result = sscanf(outputAmount, "%u", &value);
				if(result > 0)
				{
					return (uint32_t)value;
				}
			}
		}
		return INVALID_OUTPUT_AMOUNT;
	}
};

#endif /* CONVERSIONTOOL_HPP_ */
