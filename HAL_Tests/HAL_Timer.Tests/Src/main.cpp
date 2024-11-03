#include "main.hpp"

class TimerCallback : public ITimerCallback
{
	void TimerElapsed() override
	{
		control_board_led(TOGGLE);
	}
};

TimerCallback timerCallback;


int main(void)
{
	CPU_Clock_Config();

	init_board_led();

    ITimer* myTimer = CreateTimerFromMilliseconds(100);
    myTimer->RegisterCallback(
    		dynamic_cast<ITimerCallback*>(&timerCallback)
		);
    myTimer->Start();

    // TODO: iterate all timers !!

	while(1)
	{

	}
}
