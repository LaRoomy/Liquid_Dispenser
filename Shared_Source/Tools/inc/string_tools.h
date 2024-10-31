#ifndef STRING_TOOLS_H_
#define STRING_TOOLS_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>

uint8_t* string_to_u8_array(const char* str, uint32_t* length_out_param);
char* u8_array_to_string(const uint8_t* arr, uint32_t length);

#ifdef __cplusplus
 } // extern "C"
#endif

#endif /* STRING_TOOLS_H_ */
