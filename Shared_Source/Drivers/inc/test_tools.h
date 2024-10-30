#ifndef TEST_TOOLS_H_
#define TEST_TOOLS_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include <uart.h>

void init_test_tools(uint32_t sys_clk);
void assert_true(uint32_t test_val, char* opt_info);
void assert_uint_are_equal(uint32_t expected, uint32_t actual, char* opt_info);
void assert_int_are_equal(int expected, int actual, char* opt_info);
void assert_u8_arrays_are_equal(uint8_t* expected, uint8_t* actual, uint32_t size, char* opt_info);

#ifdef __cplusplus
 } // extern "C"
#endif

#endif /* TEST_TOOLS_H_ */
