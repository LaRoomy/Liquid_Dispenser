#ifndef TIMERINTERFACE_HPP_
#define TIMERINTERFACE_HPP_

class ITimerCallback
{
public:
	virtual ~ITimerCallback() = default;
	virtual void TimerElapsed() = 0;
};

class ITimer
{
public:
	virtual ~ITimer() = default;
	virtual void Start() = 0;
	virtual void Cancel() = 0;
	virtual void RegisterCallback(ITimerCallback * cb) = 0;
};

ITimer* CreateTimerFromMilliseconds(int milliseconds);
void ReleaseTimer(ITimer ** ppTimer);


#endif /* TIMERINTERFACE_HPP_ */
