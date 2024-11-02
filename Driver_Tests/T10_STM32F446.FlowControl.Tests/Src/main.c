#include "main.h"

/*
 * Test 10 - Test for the flow control system
 *
 * The flow control system consists of a pump, a valve, a flow-rate sensor and a pressure sensor
 * at the output line directly before the valve.
 * By pressing the user button different amounts of liquid are dispensed to check if the
 * device is correct calibrated and to check if the treshold values are exact.
 *
 * The pin assignment is:
 *
 * pump = 				PC0
 * valve = 				PC1
 * pressure-sense = 	PA0
 * flow-rate-sense = 	PB0
 *
 * */

void flow_control_callback(uint32_t event);

FlowControlObject flowControlObj =
{
	.flowRateSensePin = 0,
	.flowRateSensePort = GPIOB,
	.pumpPin = 0,
	.pumpPort = GPIOC,
	.valvePin = 1,
	.valvePort = GPIOC,
	.pressureADCNumber = 1,
	.pressureADCPin = 0,
	.pressureADCPort = GPIOA,
	.valveOpenThresholdValue = 2100,
	.valvePressureLostUnderCutValue = 1000,
	.apbTimerFrequency = SYS_FRQ,
	.timerNumber = 2,
	.flow_control_callback = flow_control_callback
};

uint32_t testCaseCounter = 0;

int main(void)
{
	CPU_Clock_Config();

	init_board_led();
	init_board_switch();

	flow_control_init(&flowControlObj);

	while(1)
	{
		if(get_board_switch_status_with_debounce() == PRESSED)
		{
			if(flow_control_is_active(&flowControlObj))
			{
				flow_control_stop_water_flow(&flowControlObj);
			}
			else
			{
				control_board_led(ON);

				if(testCaseCounter == 0)
				{
					flow_control_start_water_flow(&flowControlObj, 1);

					testCaseCounter++;
				}
				else if(testCaseCounter == 1)
				{
					flow_control_start_water_flow(&flowControlObj, 2);

					testCaseCounter++;
				}
				else if(testCaseCounter == 2)
				{
					flow_control_start_water_flow(&flowControlObj, 3);

					testCaseCounter++;
				}
				else if(testCaseCounter == 3)
				{
					flow_control_start_water_flow(&flowControlObj, 4);

					testCaseCounter++;
				}
				else if(testCaseCounter == 4)
				{
					flow_control_start_water_flow(&flowControlObj, 5);

					testCaseCounter++;
				}
				else if(testCaseCounter == 5)
				{
					flow_control_start_water_flow(&flowControlObj, 6);

					testCaseCounter++;
				}
				else if(testCaseCounter == 6)
				{
					flow_control_start_water_flow(&flowControlObj, QUANTITY_0_25_L);

					testCaseCounter++;
				}
				else if(testCaseCounter == 7)
				{
					flow_control_start_water_flow(&flowControlObj, QUANTITY_0_33_L);

					testCaseCounter = 0;
				}
			}
		}
	}
}

void flow_control_callback(uint32_t event)
{
	if(event == FLOWCONTROL_EVENT_STOPPED)
	{
		control_board_led(OFF);
	}
}







