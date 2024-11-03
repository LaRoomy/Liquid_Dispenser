#ifndef TIMERIMPLEMENTATION_HPP_
#define TIMERIMPLEMENTATION_HPP_

#include "TimerInterface.hpp"
#include <tim.h>

class CTimer : public ITimer
{
public:
	CTimer(uint32_t milliseconds);
	~CTimer();

	void Start() override;
	void Cancel() override;
	void RegisterCallback(ITimerCallback * cb) override;

private:
	ITimerCallback * callback;
	TimerObject timerObject;

	uint32_t millisec;
	uint32_t seconds;

	uint32_t currentSecondCounter;
	int mode;

	static void tCallback(void * param);
	bool checkCBTriggerCondition();
};


#endif /* TIMERIMPLEMENTATION_HPP_ */
