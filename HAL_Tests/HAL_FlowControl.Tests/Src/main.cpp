#include "main.hpp"

class FlowControlCallback : public IFlowControlCallback
{
public:
	void flowStopped() override
	{
		control_board_led(OFF);
	}
	void containerEmpty() override
	{
		// ...
	}
};

IFlowController * flowController;
FlowControlCallback flowControlCallback;
uint32_t testCaseCounter = 0;

int main(void)
{
	CPU_Clock_Config();
	init_board_switch();
	init_board_led();

	flowController = GetFlowControllerInstance();
	flowController->registerCallback(&flowControlCallback);

    while(1)
    {
    	if(get_board_switch_status_with_debounce() == PRESSED)
    	{
			if(flowController->isFlowActive())
			{
				flowController->stopWaterFlow();
			}
			else
			{
				control_board_led(ON);

				if(testCaseCounter == 0)
				{
					flowController->startWaterFlow(FLOWCONTROL_QUANTITY_0_25_L);

					testCaseCounter++;
				}
				else if(testCaseCounter == 1)
				{
					flowController->startWaterFlow(FLOWCONTROL_QUANTITY_0_33_L);

					testCaseCounter++;
				}
				else if(testCaseCounter == 2)
				{
					flowController->startWaterFlow(FLOWCONTROL_QUANTITY_0_5_L);

					testCaseCounter++;
				}
				else if(testCaseCounter == 3)
				{
					flowController->startWaterFlow(2);

					testCaseCounter++;
				}
				else if(testCaseCounter == 4)
				{
					flowController->startWaterFlow(4);

					testCaseCounter++;
				}
				else if(testCaseCounter == 5)
				{
					flowController->startWaterFlow(6);

					testCaseCounter = 0;
				}
			}
    	}
    }
}
