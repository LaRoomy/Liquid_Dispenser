#include "TimerInUseManager.hpp"

uint32_t TimerInUseManager::reserveFreeTimerNumber()
{
	for(uint32_t i = 0; i < 14; i++)
	{
		if(timerInUseRegister[i] == 0)
		{
			timerInUseRegister[i] = 1;
			return i + 1;
		}
	}
	return NO_TIMER_AVAILABLE;
}

void TimerInUseManager::releaseReservedTimerNumber(uint32_t tNumber)
{
	if(tNumber != 6) // do not allow to unblock the timer 6
	{
		uint32_t tIndex = tNumber - 1;
		if(tIndex < 14)
		{
			timerInUseRegister[tIndex] = 0;
		}
	}
}

bool TimerInUseManager::isTimerAvailable()
{
	for(uint32_t i = 0; i < 14; i++)
	{
		if(timerInUseRegister[i] == 0)
		{
			return true;
		}
	}
	return false;
}

uint32_t TimerInUseManager::timerInUseRegister[14] = {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0};
