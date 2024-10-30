#include "flow_control.h"
#include "tim.h"
#include "stm32f4xx_tools.h"

#define	QUANTA_PER_DECILITER					42U
#define	QUANTITY_FROM_DECILITER(q)				((q*QUANTA_PER_DECILITER)+((q*7) - 7))

static void on_flow_rate_sense_impulse(void * param);
static void flow_control_timer_callback(void * param);
static uint32_t defineQuantity(uint32_t q_val);
static flowcontrol_result_t flow_control_handle_timer_irq(FlowControlObject * self);

static flowcontrol_result_t validate_flow_control_object(FlowControlObject * self)
{
	if(self == NULL)
	{
		return FLOWCONTROL_ERROR_INVALID_PARAMETER;
	}
	if(is_valid_port(self->flowRateSensePort) == FALSE || is_valid_port(self->pumpPort) == FALSE || is_valid_port(self->valvePort) == FALSE)
	{
		return FLOWCONTROL_ERROR_INVALID_PARAMETER;
	}
	if(is_valid_pin(self->flowRateSensePin) == FALSE || is_valid_pin(self->pumpPin) == FALSE || is_valid_pin(self->valvePin) == FALSE)
	{
		return FLOWCONTROL_ERROR_INVALID_PARAMETER;
	}
	return FLOWCONTROL_OK;
}

flowcontrol_result_t flow_control_init(FlowControlObject * self)
{
	if(validate_flow_control_object(self) != FLOWCONTROL_OK)
	{
		return FLOWCONTROL_ERROR_INVALID_PARAMETER;
	}

	self->isActive = 0;
	self->max_quantity_micro_l = 0;
	self->currentFlowQuanta = 0;

	self->internal_timerObj.systemFrequency = self->apbTimerFrequency;
	self->internal_timerObj.millisecondRate = 50;
	self->internal_timerObj.timerNumber = self->timerNumber;
	self->internal_timerObj.timer_callback = flow_control_timer_callback;
	self->internal_timerObj.cb_param = self;

	self->internal_pressureSenseADCObj.adcNumber = self->pressureADCNumber;
	self->internal_pressureSenseADCObj.adcPin = self->pressureADCPin;
	self->internal_pressureSenseADCObj.adcPort = self->pressureADCPort;

	// pump pin as output
	gpio_config_port_pin(self->pumpPort, self->pumpPin, GPIO_MODE_OUTPUT);
	gpio_control_pin(self->pumpPort, self->pumpPin, LOW);

	// valve pin as output
	gpio_config_port_pin(self->valvePort, self->valvePin, GPIO_MODE_OUTPUT);
	gpio_control_pin(self->valvePort, self->valvePin, LOW);

	// flow rate sense port as digital input
	gpio_config_port_pin(self->flowRateSensePort, self->flowRateSensePin, GPIO_MODE_INPUT);

	return FLOWCONTROL_OK;
}

flowcontrol_result_t flow_control_start_water_flow(FlowControlObject * self, uint32_t quantity_dl)
{
	if(validate_flow_control_object(self) != FLOWCONTROL_OK)
	{
		return FLOWCONTROL_ERROR_INVALID_PARAMETER;
	}
	if(self->isActive)
	{
		return FLOWCONTROL_ERROR_ALREADY_ACTIVE;
	}
	self->isActive = 1;
	self->currentFlowQuanta = 0;
	self->max_quantity_micro_l = defineQuantity(quantity_dl);

	self->internal_flowCountingInterruptObj.extIntPin = self->flowRateSensePin;
	self->internal_flowCountingInterruptObj.extIntPort = self->flowRateSensePort;
	self->internal_flowCountingInterruptObj.triggerType = EXT_INT_TRIGGER_TYPE_FALLING_EDGE;
	self->internal_flowCountingInterruptObj.ext_int_callback = on_flow_rate_sense_impulse;
	self->internal_flowCountingInterruptObj.cb_param = self;

	ext_int_result_t result = external_interrupt_init(&self->internal_flowCountingInterruptObj);
	if(result != EXT_INT_OK)
	{
		return FLOWCONTROL_ERROR_INVALID_PARAMETER;
	}

	adc_init(&self->internal_pressureSenseADCObj);
	tim_interrupt_init(&self->internal_timerObj);
	gpio_control_pin(self->pumpPort, self->pumpPin, HIGH);

	return FLOWCONTROL_OK;
}

flowcontrol_result_t flow_control_stop_water_flow(FlowControlObject * self)
{
	external_interrupt_deinit(&self->internal_flowCountingInterruptObj);
	tim_deinit(&self->internal_timerObj);
	adc_deinit(&self->internal_pressureSenseADCObj);
	gpio_control_pin(self->valvePort, self->valvePin, LOW);
	gpio_control_pin(self->pumpPort, self->pumpPin, LOW);

	self->isActive = 0;

	if(self->flow_control_callback != NULL)
	{
		self->flow_control_callback(FLOWCONTROL_EVENT_STOPPED);
	}

	return FLOWCONTROL_OK;
}

BOOL is_valve_open(FlowControlObject * self)
{
	return ((self->valvePort->ODR & (1U<<self->valvePin)) != 0) ? TRUE : FALSE;
}

static flowcontrol_result_t messure_flow_pressure(FlowControlObject * self)
{
	uint32_t pressureValue;

	adc_result_t res = adc_read_average(&self->internal_pressureSenseADCObj, 5, &pressureValue);
	if(res == ADC_OK)
	{
		if(pressureValue > (self->valveOpenThresholdValue + 30U))
		{
			// open valve
			gpio_control_pin(self->valvePort, self->valvePin, HIGH);
		}
		else
		{
			if(is_valve_open(self) == TRUE)
			{
				if(pressureValue < (self->valvePressureLostUnderCutValue))
				{
					flow_control_stop_water_flow(self);

					// enter error state (no pressure)

					if(self->flow_control_callback != NULL)
					{
						self->flow_control_callback(FLOWCONTROL_EVENT_PRESSURE_LOST);
					}
					return FLOWCONTROL_ERROR_PRESSURE_LOST;
				}
			}
		}
		return FLOWCONTROL_OK;
	}
	else
	{
		return FLOWCONTROL_ERROR_IO;
	}
}

static uint32_t defineQuantity(uint32_t q_val)
{
	if(q_val == QUANTITY_INDETERMINATE)
	{
		return q_val;
	}
	else if(q_val == QUANTITY_0_25_L)
	{
		return QUANTITY_FROM_DECILITER(2) + (QUANTITY_FROM_DECILITER(1) / 2);
	}
	else if(q_val == QUANTITY_0_33_L)
	{
		return QUANTITY_FROM_DECILITER(3) + (QUANTITY_FROM_DECILITER(1) / 3);
	}
	else if(q_val == QUANTITY_0_5_L)
	{
		return QUANTITY_FROM_DECILITER(5);
	}
	else if(q_val == QUANTITY_0_75_L)
	{
		return QUANTITY_FROM_DECILITER(7) + (QUANTITY_FROM_DECILITER(1) / 2);
	}
	else if(q_val == QUANTITY_1_L)
	{
		return QUANTITY_FROM_DECILITER(10);
	}
	else
	{
		return QUANTITY_FROM_DECILITER(q_val);
	}
}

flowcontrol_result_t flow_control_handle_timer_irq(FlowControlObject * self)
{
	if(self->isActive)
	{
		return messure_flow_pressure(self);
	}
	return FLOWCONTROL_OK;
}

BOOL flow_control_is_active(FlowControlObject * self)
{
	return self->isActive == 0 ? FALSE : TRUE;
}

static void on_flow_rate_sense_impulse(void * param)
{
	FlowControlObject * fcObj = (FlowControlObject*)param;
	if(fcObj != NULL)
	{
		fcObj->currentFlowQuanta++;

		if(fcObj->max_quantity_micro_l != QUANTITY_INDETERMINATE)
		{
			if(fcObj->currentFlowQuanta >= fcObj->max_quantity_micro_l)
			{
				flow_control_stop_water_flow(fcObj);
			}
		}
	}
}

static void flow_control_timer_callback(void * param)
{
	if(param != NULL)
	{
		FlowControlObject * fcObj = (FlowControlObject*)param;

		flowcontrol_result_t res = flow_control_handle_timer_irq(fcObj);
		if(res != FLOWCONTROL_OK)
		{
			// handle error
		}
	}
}



