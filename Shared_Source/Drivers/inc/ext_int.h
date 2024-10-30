#ifndef EXT_INT_H_
#define EXT_INT_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stm32f4xx.h>

#define		EXT_INT_OK						0
#define		EXT_INT_ERROR_INVALID_ARGUMENT	(-22)

#define		EXT_INT_TRIGGER_TYPE_FALLING_EDGE	0
#define		EXT_INT_TRIGGER_TYPE_RISING_EDGE	1

typedef struct
{
	GPIO_TypeDef * extIntPort;
	uint32_t extIntPin;
	uint32_t triggerType;
	void (*ext_int_callback)(void * param);
	void * cb_param;

}ExternalInterruptObject;

typedef int ext_int_result_t;

ext_int_result_t external_interrupt_init(ExternalInterruptObject * self);
ext_int_result_t external_interrupt_deinit(ExternalInterruptObject * self);

#ifdef __cplusplus
 } // extern "C"
#endif

#endif /* EXT_INT_H_ */
