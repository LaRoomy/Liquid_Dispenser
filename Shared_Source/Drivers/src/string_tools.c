#include "string_tools.h"
#include <string.h>
#include <malloc.h>

uint8_t* string_to_u8_array(const char* str, uint32_t* length_out_param)
{
	uint8_t* buffer = NULL;

	if(str != NULL)
	{
		uint32_t len = strlen(str);
		if(len > 0)
		{
			*length_out_param = len;

			buffer = malloc((len*sizeof(uint8_t)));
			if(buffer != NULL)
			{
				for(uint32_t i = 0; i < len; i++)
				{
					buffer[i] = (uint8_t)str[i];
				}
			}
		}
	}
	return buffer;
}

char* u8_array_to_string(const uint8_t* arr, uint32_t length)
{
	char* buffer = NULL;

	if(arr != NULL && length > 0)
	{
		buffer = malloc(((length+1)*sizeof(char)));
		if(buffer != NULL)
		{
			for(uint32_t i = 0; i < length; i++)
			{
				buffer[i] = (char)arr[i];
			}
			buffer[length] = '\0';
		}
	}
	return buffer;
}


