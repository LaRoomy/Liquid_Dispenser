#ifndef DELAY_H_
#define DELAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*
 * NOTE: the delay function is using timer 6 !
 * */
void delay_ms(uint32_t milliseconds, uint32_t sys_clk);

#ifdef __cplusplus
      } // extern "C"
#endif

#endif /* DELAY_H_ */
