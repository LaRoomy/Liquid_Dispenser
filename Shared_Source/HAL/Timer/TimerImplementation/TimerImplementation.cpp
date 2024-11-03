#include "TimerImplementation.hpp"
#include "TimerInUseManager.hpp"
#include <stddef.h>

constexpr int LongTimerModeNone = 0;
constexpr int LongTimerModeMsecPart = 1;

CTimer::CTimer(uint32_t milliseconds)
	: callback(nullptr), seconds(0), currentSecondCounter(0), mode(LongTimerModeNone)
{
	if(milliseconds > 0)
	{
		uint32_t freeTimerNumber =
				TimerInUseManager::reserveFreeTimerNumber();

		if(freeTimerNumber != NO_TIMER_AVAILABLE)
		{
			this->timerObject.timerNumber = freeTimerNumber;

			this->timerObject.systemFrequency = MPU_CLOCK;
			this->timerObject.cb_param = this;
			this->timerObject.timer_callback = CTimer::tCallback;

			if(milliseconds > 1000)
			{
				this->seconds = (milliseconds / 1000);
				this->millisec = (milliseconds % 1000);
				this->timerObject.millisecondRate = 1000;
			}
			else
			{
				this->millisec = milliseconds;
				this->timerObject.millisecondRate = milliseconds;
			}
		}
		else
		{
			this->timerObject.timerNumber = 0;
		}
	}
}

CTimer::~CTimer()
{
	tim_deinit(&this->timerObject);
	TimerInUseManager::releaseReservedTimerNumber(this->timerObject.timerNumber);
}

void CTimer::Start()
{
	tim_interrupt_init(&this->timerObject);
}

void CTimer::Cancel()
{
	tim_deinit(&this->timerObject);
	this->mode = LongTimerModeNone;
	this->currentSecondCounter = 0;
}

void CTimer::RegisterCallback(ITimerCallback * cb)
{
	this->callback = cb;
}

void CTimer::tCallback(void * param)
{
	CTimer * _this_ = (CTimer*)param;
	if(_this_ != NULL)
	{
		if(_this_->checkCBTriggerCondition() == true)
		{
			if(_this_->callback != nullptr)
			{
				_this_->callback->TimerElapsed();
			}
		}
	}
}

bool CTimer::checkCBTriggerCondition()
{
	if(this->seconds > 0)
	{
		if(this->mode == LongTimerModeMsecPart)
		{
			this->Cancel();
			this->mode = LongTimerModeNone;
			this->currentSecondCounter = 0;
			this->timerObject.millisecondRate = 1000;
			this->Start();

			return true;
		}

		this->currentSecondCounter++;

		if(this->currentSecondCounter == this->seconds)
		{
			if(this->millisec == 0)
			{
				this->currentSecondCounter = 0;
				return true;
			}
			else
			{
				this->Cancel();
				this->mode = LongTimerModeMsecPart;
				this->timerObject.millisecondRate = this->millisec;
				this->Start();

				return false;
			}
		}
		return false;
	}
	else
	{
		return true;
	}
}

ITimer* CreateTimerFromMilliseconds(int milliseconds)
{
	if(TimerInUseManager::isTimerAvailable())
	{
		return new CTimer(milliseconds);
	}
	else
	{
		return nullptr;
	}
}

void ReleaseTimer(ITimer ** ppTimer)
{
	if(*ppTimer != nullptr)
	{
		delete *ppTimer;
		*ppTimer = nullptr;
	}
}

