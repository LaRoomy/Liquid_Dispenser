#ifndef TIMERINUSEMANAGER_HPP_
#define TIMERINUSEMANAGER_HPP_

#include <stdint.h>

constexpr uint32_t NO_TIMER_AVAILABLE = 15;

class TimerInUseManager
{
public:
	static uint32_t reserveFreeTimerNumber();
	static void releaseReservedTimerNumber(uint32_t tNumber);
	static bool isTimerAvailable();
private:
	static uint32_t timerInUseRegister[14];
};


#endif /* TIMERINUSEMANAGER_HPP_ */
