#ifndef TIM_H_
#define TIM_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>

#define		TIMER_OK						0
#define		TIMER_ERROR_INVALID_PARAMETER	(-22)
#define		TIMER_ERROR_ALREADY_IN_USE		(-114)
#define		TIMER_ERROR_UNEXPECTED			(-134)

typedef int timer_result_t;

typedef struct
{
	uint32_t timerNumber;// 1 - 14
	uint32_t millisecondRate;// milliseconds
	uint32_t systemFrequency;// Hz
	void (*timer_callback)(void * param);
	void * cb_param;

}TimerObject;

timer_result_t tim_interrupt_init(TimerObject * self);
timer_result_t tim_deinit(TimerObject * self);

#ifdef __cplusplus
 } // extern "C"
#endif

#endif /* TIM_H_ */
