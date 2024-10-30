#include "delay.h"
#include <stm32f4xx.h>

#define	TIM_CEN	(1U<<0)
#define TIM_UIF	(1U<<0)
#define	TIM6_EN	(1U<<4)

void delay_ms(uint32_t milliseconds, uint32_t sys_clk)
{
	uint32_t ticksPerMillisecond = sys_clk / 4000;

	// save previous timer state
	uint32_t TIM6_CR1 = TIM6->CR1;
	uint32_t TIM6_CR2 = TIM6->CR2;
	uint32_t TIM6_DIER = TIM6->DIER;
	uint32_t TIM6_PSC = TIM6->PSC;
	uint32_t TIM6_ARR = TIM6->ARR;
	TIM6->DIER = 0;
	TIM6->CR1 = 0;
	TIM6->CR2 = 0;
	TIM6->SR = 0;
	TIM6->CNT = 0;

	// save previous clock source enabled state
	uint32_t timerWasEnabled = RCC->APB1ENR & (TIM6_EN);

	// enable clock source for timer 6
	RCC->APB1ENR |= (TIM6_EN);

	// set prescaler 4 to cover the highest possible frequency
	TIM6->PSC = ((uint16_t)3);

	// set auto reload register
	TIM6->ARR = ((uint16_t)(ticksPerMillisecond - 1));

	// enable counter
	TIM6->CR1 |= (TIM_CEN);

	// count milliseconds
	uint32_t milliCnt = 0;
	while(1)
	{
		if(TIM6->SR & TIM_UIF)
		{
			TIM6->SR &= ~(TIM_UIF);
			milliCnt++;
		}
		if(milliCnt == milliseconds)
		{
			break;
		}
	}

	// disable counter
	TIM6->CR1 &= ~(TIM_CEN);

	// disable clock source if applicable
	if(timerWasEnabled == 0)
	{
		RCC->APB1ENR &= ~(TIM6_EN);
	}

	// recover old timer state
	TIM6->CR2 = TIM6_CR2;
	TIM6->DIER = TIM6_DIER;
	TIM6->PSC = TIM6_PSC;
	TIM6->ARR = TIM6_ARR;
	TIM6->SR = 0;
	TIM6->CR1 = TIM6_CR1;
}


