#include "stm32f4xx.h"
#include "tim.h"
#include "common.h"

#define	CR1_CEN		(1U<<0)
#define	DIER_UIE	(1U<<0)

#define	SR_UIF	(1U<<0)

typedef struct
{
	void (*tim_callback)(void * opt_param);
	uint32_t timerNumber;
	void * opt_param;

}TimerStorageObject;

static TimerStorageObject timerStorage[14];
static uint32_t storageInitialized = 0;

static timer_result_t validate_timer_object(TimerObject * self);
static TIM_TypeDef * get_timer_module_from_number(uint32_t timerNumber);
static uint32_t timer_bit_for_timer_number(uint32_t timerNumber);
static IRQn_Type irq_vect_for_timer_number(uint32_t timerNumber);
static timer_result_t is_valid_timer_number(uint32_t timerNumber);
static timer_result_t enable_clock_access_to_timer(TimerObject * self);
static timer_result_t disable_clock_access_to_timer(TimerObject * self);

static timer_result_t tim_on_interrupt(uint32_t timerNumber)
{
	TIM_TypeDef * TIMx = get_timer_module_from_number(timerNumber);
	if(TIMx != NULL)
	{
		/* clear the UIF */
		TIMx->SR &= ~SR_UIF;

		// find callback for timer object
		for(uint32_t i = 0; i < 14; i++)
		{
			if(timerStorage[i].timerNumber == timerNumber)
			{
				timerStorage[i].tim_callback(timerStorage[i].opt_param);
				break;
			}
		}
		return TIMER_OK;
	}
	else
	{
		return TIMER_ERROR_INVALID_PARAMETER;
	}
}

timer_result_t tim_interrupt_init(TimerObject * self)
{
	if(validate_timer_object(self) != TIMER_OK)
	{
		return TIMER_ERROR_INVALID_PARAMETER;
	}

	if(storageInitialized == 0)
	{
		for(uint32_t i = 0; i < 14; i++)
		{
			timerStorage[i].tim_callback = NULL;
			timerStorage[i].timerNumber = 0;
			timerStorage[i].opt_param = NULL;
		}
		storageInitialized = 1;
	}

	// get related timer register
	TIM_TypeDef * TIMx = get_timer_module_from_number(self->timerNumber);

	if((TIMx->CR1 & (CR1_CEN)) != 0)
	{
		return TIMER_ERROR_ALREADY_IN_USE;
	}

	// calculate prescaler value
	uint32_t psv = (self->systemFrequency / 10000);

	// enable clock access to timer X
	if(enable_clock_access_to_timer(self) != TIMER_OK)
	{
		return TIMER_ERROR_UNEXPECTED;
	}

	// set prescaler value
	TIMx->PSC = psv - 1;		// sys_freq / psv = 10000

	// set auto-reload value
	if(self->millisecondRate > 10000)
	{
		TIMx->ARR = 1; // do not allow frequencies over 10000Hz
	}
	else
	{
		TIMx->ARR = ((10000 * self->millisecondRate) / 1000) - 1; // millisecond delay
	}

	// clear the timer counter
	TIMx->CNT = 0;

	// save timer data
	if(self->timer_callback != NULL)
	{
		for(uint32_t i = 0; i < 14; i++)
		{
			if(timerStorage[i].tim_callback == NULL)
			{
				timerStorage[i].tim_callback = self->timer_callback;
				timerStorage[i].timerNumber = self->timerNumber;
				timerStorage[i].opt_param = self->cb_param;
				break;
			}
		}
	}

	// enable the timer
	TIMx->CR1 = (CR1_CEN);

	/* enable TIM interrupt */
	TIMx->DIER |= (DIER_UIE);

	/* enable TIM interrupt in NVIC */
	NVIC_EnableIRQ(
			irq_vect_for_timer_number(self->timerNumber)
		);

	return TIMER_OK;
}

timer_result_t tim_deinit(TimerObject * self)
{
	if(validate_timer_object(self) != TIMER_OK)
	{
		return TIMER_ERROR_INVALID_PARAMETER;
	}

	// disable interrupt
	NVIC_DisableIRQ(
			irq_vect_for_timer_number(self->timerNumber)
		);

	// get related timer register
	TIM_TypeDef * TIMx = get_timer_module_from_number(self->timerNumber);

	// reset timer registers
	TIMx->PSC = 0;
	TIMx->ARR = 0;
	TIMx->CNT = 0;
	TIMx->CR1 = 0;
	TIMx->DIER = 0;

	// disable clock access to timer X
	disable_clock_access_to_timer(self);

	// erase timer data
	for(uint32_t i = 0; i < 14; i++)
	{
		if(timerStorage[i].timerNumber == self->timerNumber)
		{
			timerStorage[i].tim_callback = NULL;
			timerStorage[i].timerNumber = 0;
			break;
		}
	}
	return TIMER_OK;
}

static timer_result_t enable_clock_access_to_timer(TimerObject * self)
{
	if(self->timerNumber == 1 || self->timerNumber == 8 || self->timerNumber == 9 || self->timerNumber == 10 || self->timerNumber == 11)
	{
		// apb2
		RCC->APB2ENR |= (1U<<(timer_bit_for_timer_number(self->timerNumber)));
	}
	else if(self->timerNumber == 2 || self->timerNumber == 3 || self->timerNumber == 4 || self->timerNumber == 5
			|| self->timerNumber == 6 || self->timerNumber == 7 || self->timerNumber == 12 || self->timerNumber == 13 || self->timerNumber == 14)
	{
		// apb1
		RCC->APB1ENR |= (1U<<(timer_bit_for_timer_number(self->timerNumber)));
	}
	else
	{
		return TIMER_ERROR_INVALID_PARAMETER;
	}
	return TIMER_OK;
}

static timer_result_t disable_clock_access_to_timer(TimerObject * self)
{
	if(self->timerNumber == 1 || self->timerNumber == 8 || self->timerNumber == 9 || self->timerNumber == 10 || self->timerNumber == 11)
	{
		// apb2
		RCC->APB2ENR &= ~(1U<<(timer_bit_for_timer_number(self->timerNumber)));
	}
	else if(self->timerNumber == 2 || self->timerNumber == 3 || self->timerNumber == 4 || self->timerNumber == 5
			|| self->timerNumber == 6 || self->timerNumber == 7 || self->timerNumber == 12 || self->timerNumber == 13 || self->timerNumber == 14)
	{
		// apb1
		RCC->APB1ENR &= ~(1U<<(timer_bit_for_timer_number(self->timerNumber)));
	}
	else
	{
		return TIMER_ERROR_INVALID_PARAMETER;
	}
	return TIMER_OK;
}

static timer_result_t validate_timer_object(TimerObject * self)
{
	if(self == NULL)
	{
		return TIMER_ERROR_INVALID_PARAMETER;
	}
	if(self->systemFrequency == 0 || self->millisecondRate == 0 || self->millisecondRate > 1000U || (is_valid_timer_number(self->timerNumber) != TIMER_OK))
	{
		return TIMER_ERROR_INVALID_PARAMETER;
	}
	return TIMER_OK;
}

static TIM_TypeDef * get_timer_module_from_number(uint32_t timerNumber)
{
	switch(timerNumber)
	{
	case 1:
		return TIM1;
	case 2:
		return TIM2;
	case 3:
		return TIM3;
	case 4:
		return TIM4;
	case 5:
		return TIM5;
	case 6:
		return TIM6;
	case 7:
		return TIM7;
	case 8:
		return TIM8;
	case 9:
		return TIM9;
	case 10:
		return TIM10;
	case 11:
		return TIM11;
	case 12:
		return TIM12;
	case 13:
		return TIM13;
	case 14:
		return TIM14;
	default:
		return NULL;
	}
}

static uint32_t timer_bit_for_timer_number(uint32_t timerNumber)
{
	switch(timerNumber)
	{
	case 1:
		return 0U;
	case 2:
		return 0U;
	case 3:
		return 1U;
	case 4:
		return 2U;
	case 5:
		return 3U;
	case 6:
		return 4U;
	case 7:
		return 5U;
	case 8:
		return 1U;
	case 9:
		return 16U;
	case 10:
		return 17U;
	case 11:
		return 18U;
	case 12:
		return 6U;
	case 13:
		return 7U;
	case 14:
		return 8U;
	default:
		return 0U;
	}
}

static IRQn_Type irq_vect_for_timer_number(uint32_t timerNumber)
{
	switch(timerNumber)
	{
	case 1:
		return TIM1_UP_TIM10_IRQn;
	case 2:
		return TIM2_IRQn;
	case 3:
		return TIM3_IRQn;
	case 4:
		return TIM4_IRQn;
	case 5:
		return TIM5_IRQn;
	case 6:
		return TIM6_DAC_IRQn;
	case 7:
		return TIM7_IRQn;
	case 8:
		return TIM8_UP_TIM13_IRQn;
	case 9:
		return TIM1_BRK_TIM9_IRQn;
	case 10:
		return TIM1_UP_TIM10_IRQn;
	case 11:
		return TIM1_TRG_COM_TIM11_IRQn;
	case 12:
		return TIM8_BRK_TIM12_IRQn;
	case 13:
		return TIM8_UP_TIM13_IRQn;
	case 14:
		return TIM8_TRG_COM_TIM14_IRQn;
	default:
		return 0U;
	}
}

static timer_result_t is_valid_timer_number(uint32_t timerNumber)
{
	if(timerNumber > 0 && timerNumber < 15)
	{
		return TIMER_OK;
	}
	else
	{
		return TIMER_ERROR_INVALID_PARAMETER;
	}
}

void TIM1_UP_TIM10_IRQHandler()
{
	if((TIM1->SR & SR_UIF) != 0)
	{
		tim_on_interrupt(1);
	}
	else if((TIM10->SR & SR_UIF) != 0)
	{
		tim_on_interrupt(10);
	}
}

void TIM2_IRQHandler()
{
	tim_on_interrupt(2);
}

void TIM3_IRQHandler()
{
	tim_on_interrupt(3);
}

void TIM4_IRQHandler()
{
	tim_on_interrupt(4);
}

void TIM5_IRQHandler()
{
	tim_on_interrupt(5);
}

void TIM6_DAC_IRQHandler()
{
	tim_on_interrupt(6);
}

void TIM7_IRQHandler()
{
	tim_on_interrupt(7);
}

void TIM1_BRK_TIM9_IRQHandler()
{
	if((TIM9->SR & SR_UIF) != 0) {
		tim_on_interrupt(9);
	}
}

void TIM1_TRG_COM_TIM11_IRQHandler()
{
	if((TIM11->SR & SR_UIF) != 0) {
		tim_on_interrupt(11);
	}
}

void TIM8_BRK_TIM12_IRQHandler()
{
	if((TIM12->SR & SR_UIF) != 0) {
		tim_on_interrupt(12);
	}
}

void TIM8_UP_TIM13_IRQHandler()
{
	if((TIM8->SR & SR_UIF) != 0)
	{
		tim_on_interrupt(8);
	}
	else if((TIM13->SR & SR_UIF) != 0)
	{
		tim_on_interrupt(13);
	}
}

void TIM8_TRG_COM_TIM14_IRQHandler()
{
	if((TIM14->SR & SR_UIF) != 0) {
		tim_on_interrupt(14);
	}
}

