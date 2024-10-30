#include "uint8_tools.h"
#include <stddef.h>

int uint8_to_char_array(uint8_t * uArr, char * cArr, uint32_t size)
{
	if(size == 0 || uArr == NULL || cArr == NULL)
	{
		return -1;
	}
	else
	{
		for(uint32_t i = 0; i < size; i++)
		{
			cArr[i] = (char)uArr[i];
		}
		return 0;
	}
}

int char_to_uint8_array(const char * cArr, uint8_t * uArr, uint32_t size)
{
	if(size == 0 || uArr == NULL || cArr == NULL)
	{
		return -1;
	}
	else
	{
		for(uint32_t i = 0; i < size; i++)
		{
			uArr[i] = (uint8_t)cArr[i];
		}
		return 0;
	}
}

