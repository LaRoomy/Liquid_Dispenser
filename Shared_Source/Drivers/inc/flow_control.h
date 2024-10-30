#ifndef FLOW_CONTROL_H_
#define FLOW_CONTROL_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f4xx.h"
#include "common.h"
#include "tim.h"
#include "adc.h"
#include "ext_int.h"

#define	FLOWCONTROL_OK							0
#define	FLOWCONTROL_ERROR_IO					(-5)
#define	FLOWCONTROL_ERROR_ALREADY_ACTIVE		(-16)
#define	FLOWCONTROL_ERROR_INVALID_PARAMETER		(-22)
#define	FLOWCONTROL_ERROR_PRESSURE_LOST			(-140)

#define	QUANTITY_INDETERMINATE					(0xffffff0)
#define QUANTITY_0_25_L							(0xffffff1)
#define	QUANTITY_0_33_L							(0xffffff2)
#define	QUANTITY_0_5_L							(0xffffff3)
#define	QUANTITY_0_75_L							(0xffffff4)
#define QUANTITY_1_L							(0xffffff5)

#define	FLOWCONTROL_EVENT_STOPPED				0
#define	FLOWCONTROL_EVENT_PRESSURE_LOST			2

typedef int flowcontrol_result_t;

typedef struct
{
	GPIO_TypeDef * pumpPort;
	uint32_t pumpPin;
	GPIO_TypeDef * valvePort;
	uint32_t valvePin;
	GPIO_TypeDef * flowRateSensePort;
	uint32_t pressureADCNumber;
	uint32_t pressureADCPin;
	GPIO_TypeDef * pressureADCPort;
	uint32_t flowRateSensePin;
	uint32_t isActive;
	uint32_t valveOpenThresholdValue;
	uint32_t valvePressureLostUnderCutValue;
	uint32_t max_quantity_micro_l;
	uint32_t currentFlowQuanta;
	uint32_t apbTimerFrequency;
	uint32_t timerNumber;

	TimerObject internal_timerObj;
	ADCObject internal_pressureSenseADCObj;
	ExternalInterruptObject internal_flowCountingInterruptObj;

	void (*flow_control_callback)(uint32_t event);

}FlowControlObject;

flowcontrol_result_t flow_control_init(FlowControlObject * self);
flowcontrol_result_t flow_control_start_water_flow(FlowControlObject * self, uint32_t quantity_dl);
flowcontrol_result_t flow_control_stop_water_flow(FlowControlObject * self);
BOOL flow_control_is_active(FlowControlObject * self);

#ifdef __cplusplus
 } // extern "C"
#endif

#endif /* FLOW_CONTROL_H_ */
