#include "cpu_clock.h"
#include "stm32f4xx.h"

void CPU_Clock_Config()
{
	// CONFIGURATION FOR 48MHz system clock

	// start ext. crystal osc
	RCC->CR |= (RCC_CR_HSEON);

	// wait for hse to be ready
	while(!(RCC->CR & (RCC_CR_HSERDY))){}

	// config pll

	// division factor for main pll 5:0 -> 000100 (/4)
	RCC->PLLCFGR &= ~(1U<<0);
	RCC->PLLCFGR &= ~(1U<<1);
	RCC->PLLCFGR |= (1U<<2);
	RCC->PLLCFGR &= ~(1U<<3);
	RCC->PLLCFGR &= ~(1U<<4);
	RCC->PLLCFGR &= ~(1U<<5);

	// multiplicaton factor 14:6 -> 001100000 (X96)
	RCC->PLLCFGR &= ~(1U<<6);
	RCC->PLLCFGR &= ~(1U<<7);
	RCC->PLLCFGR &= ~(1U<<8);
	RCC->PLLCFGR &= ~(1U<<9);
	RCC->PLLCFGR &= ~(1U<<10);
	RCC->PLLCFGR |= (1U<<11);
	RCC->PLLCFGR |= (1U<<12);
	RCC->PLLCFGR &= ~(1U<<13);
	RCC->PLLCFGR &= ~(1U<<14);

	// division factor for main system clock 17:16 -> 01 (/4)
	RCC->PLLCFGR |= (1U<<16);
	RCC->PLLCFGR &= ~(1U<<17);

	// pll source as HSE 22 -> 1
	RCC->PLLCFGR |= (1U<<22);

	// enable pll
	RCC->CR |= (RCC_CR_PLLON);

	// enable security check
	RCC->CR |= (RCC_CR_CSSON);

	// wait for pll to be ready
	while(!(RCC->CR & (RCC_CR_PLLRDY))){}

	// set pll as system clock source
	RCC->CFGR &= ~(1U<<0);
	RCC->CFGR |= (1U<<1);

	// check system clock source
	while (!(RCC->CFGR & (RCC_CFGR_SWS_PLL))){}

	// flash latency wait state = 1
	FLASH->ACR = 0x01;
}



