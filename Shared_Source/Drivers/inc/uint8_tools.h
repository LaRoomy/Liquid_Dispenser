#ifndef UINT8_TOOLS_H_
#define UINT8_TOOLS_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>

int uint8_to_char_array(uint8_t * uArr, char * cArr, uint32_t size);
int char_to_uint8_array(const char * cArr, uint8_t * uArr, uint32_t size);

#ifdef __cplusplus
 } // extern "C"
#endif

#endif // UINT8_TOOLS_H_
