#ifndef NUCLEOF446_IO_H_
#define NUCLEOF446_IO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <common.h>

void init_board_led();
void control_board_led(int status);

void init_board_switch();
uint32_t get_board_switch_status();
uint32_t get_board_switch_status_with_debounce();

#ifdef __cplusplus
      } // extern "C"
#endif

#endif /* NUCLEOF446_IO_H_ */
