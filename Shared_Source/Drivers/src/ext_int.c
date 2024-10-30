#include "ext_int.h"
#include <stm32f4xx.h>
#include <stm32f4xx_tools.h>

#define SYSCFGEN	(1U<<14)

typedef struct
{
	GPIO_TypeDef * port;
	uint32_t pin;
	void (*exti_callback)(void * param);
	void * opt_param;

}EXTIRegisterObject;

static EXTIRegisterObject extiRegister[16];
static uint32_t regInitialized = 0;

static void sys_cfg_set_port_for_exti(GPIO_TypeDef * port, uint32_t pin);
static IRQn_Type get_irq_vect_for_pin(uint32_t pin);
static void construct_obj_and_trigger_callback(uint32_t pin);
static void sys_cfg_erase_port_for_exti(GPIO_TypeDef * port, uint32_t pin);

void EXTI0_IRQHandler()
{
	EXTI->PR |= (1U<<0);
	construct_obj_and_trigger_callback(0);
}

void EXTI1_IRQHandler()
{
	EXTI->PR |= (1U<<1);
	construct_obj_and_trigger_callback(1);
}

void EXTI2_IRQHandler()
{
	EXTI->PR |= (1U<<2);
	construct_obj_and_trigger_callback(2);
}

void EXTI3_IRQHandler()
{
	EXTI->PR |= (1U<<3);
	construct_obj_and_trigger_callback(3);
}

void EXTI4_IRQHandler()
{
	EXTI->PR |= (1U<<4);
	construct_obj_and_trigger_callback(4);
}

void EXTI9_5_IRQHandler()
{
	for(uint32_t i = 5; i < 10; i++)
	{
		if((EXTI->PR & (1U<<i)) != 0)
		{
			EXTI->PR |= (1U<<i); // clear flag
			construct_obj_and_trigger_callback(i);
			break;
		}
	}
}

void EXTI15_10_IRQHandler()
{
	for(uint32_t i = 10; i < 16; i++)
	{
		if((EXTI->PR & (1U<<i)) != 0)
		{
			EXTI->PR |= (1U<<i); // clear flag
			construct_obj_and_trigger_callback(i);
			break;
		}
	}
}

static ext_int_result_t validate_self(ExternalInterruptObject * self)
{
	if(self == NULL)
	{
		return EXT_INT_ERROR_INVALID_ARGUMENT;
	}
	if(is_valid_port(self->extIntPort) == FALSE || is_valid_pin(self->extIntPin) == FALSE)
	{
		return EXT_INT_ERROR_INVALID_ARGUMENT;
	}
	if(self->triggerType > 1)
	{
		return EXT_INT_ERROR_INVALID_ARGUMENT;
	}
	return EXT_INT_OK;
}

ext_int_result_t external_interrupt_init(ExternalInterruptObject * self)
{
	if(validate_self(self) != EXT_INT_OK)
	{
		return EXT_INT_ERROR_INVALID_ARGUMENT;
	}

	// initialize callback register (if required)
	if(regInitialized == 0)
	{
		regInitialized = 1;
		for(uint32_t i = 0; i < 16; i++)
		{
			extiRegister[i].port = NULL;
		}
	}

	/* disable global interrupts */
	//__disable_irq();

	gpio_config_port_pin(self->extIntPort, self->extIntPin, GPIO_MODE_INPUT);

	/* enable clock access to SYSCFG */
	RCC->APB2ENR |= SYSCFGEN;

	/* enable port in sys config */
	sys_cfg_set_port_for_exti(self->extIntPort, self->extIntPin);

	/* unmask EXTI (enable interrupt) */
	EXTI->IMR |= (1U<<self->extIntPin);

	/* select trigger type */
	if(self->triggerType == EXT_INT_TRIGGER_TYPE_FALLING_EDGE)
	{
		/* select falling edge trigger */
		EXTI->FTSR |= (1U<<self->extIntPin);
	}
	else
	{
		/* select rising edge trigger */
		EXTI->RTSR |= (1U<<self->extIntPin);
	}

	/* register callback for specific interrupt */
	if(self->ext_int_callback != NULL)
	{
		for(uint32_t regNum = 0; regNum < 16; regNum++)
		{
			if(extiRegister[regNum].port == NULL)
			{
				extiRegister[regNum].exti_callback = self->ext_int_callback;
				extiRegister[regNum].pin = self->extIntPin;
				extiRegister[regNum].port = self->extIntPort;
				extiRegister[regNum].opt_param = self->cb_param;
				break;
			}
		}
	}

	// make sure the priority is not zero (for compatibility with a RTOS)
	NVIC_SetPriority(get_irq_vect_for_pin(self->extIntPin), 3U);

	NVIC_EnableIRQ(
		get_irq_vect_for_pin(self->extIntPin)
	);

	/* enable global interrups */
	//__enable_irq();

	return EXT_INT_OK;
}

ext_int_result_t external_interrupt_deinit(ExternalInterruptObject * self)
{
	if(validate_self(self) != EXT_INT_OK)
	{
		return EXT_INT_ERROR_INVALID_ARGUMENT;
	}

	/* disable global interrupts */
	//__disable_irq();

	NVIC_DisableIRQ(
		get_irq_vect_for_pin(self->extIntPin)
	);

	/* erase trigger type */
	EXTI->FTSR &= ~(1U<<self->extIntPin);
	EXTI->RTSR &= ~(1U<<self->extIntPin);

	/* mask EXTI (disable interrupt) */
	EXTI->IMR &= ~(1U<<self->extIntPin);

	sys_cfg_erase_port_for_exti(self->extIntPort, self->extIntPin);

	/* unregister callback for specific interrupt */
	for(uint32_t regNum = 0; regNum < 16; regNum++)
	{
		if(extiRegister[regNum].port == self->extIntPort && extiRegister[regNum].pin == self->extIntPin)
		{
			extiRegister[regNum].exti_callback = NULL;
			extiRegister[regNum].pin = 0;
			extiRegister[regNum].port = NULL;
			extiRegister[regNum].opt_param = NULL;
			break;
		}
	}

	/* enable global interrups */
	//__enable_irq();

	return EXT_INT_OK;
}

static uint32_t sys_cfg_exti_cr_reg_index_from_pin(uint32_t pin)
{
	if(pin == 0 || pin == 1 || pin == 2 || pin == 3)
	{
		return 0;
	}
	else if(pin == 4 || pin == 5 || pin == 6 || pin == 7)
	{
		return 1;
	}
	else if(pin == 8 || pin == 9 || pin == 10 || pin == 11)
	{
		return 2;
	}
	else if(pin == 12 || pin == 13 || pin == 14 || pin == 15)
	{
		return 3;
	}
	else
	{
		return 0;
	}
}

static uint32_t sys_cfg_exti_cr_start_bit_from_pin(uint32_t pin)
{
	switch(pin)
	{
	case 0:
		return 0;
	case 1:
		return 4;
	case 2:
		return 8;
	case 3:
		return 12;
	case 4:
		return 0;
	case 5:
		return 4;
	case 6:
		return 8;
	case 7:
		return 12;
	case 8:
		return 0;
	case 9:
		return 4;
	case 10:
		return 8;
	case 11:
		return 12;
	case 12:
		return 0;
	case 13:
		return 4;
	case 14:
		return 8;
	case 15:
		return 12;
	default:
		return 0;
	}
}

static uint32_t sys_cfg_exti_value_for_port(GPIO_TypeDef * port)
{
	if(port == GPIOA){
		return 0;
	}
	else if(port == GPIOB){
		return 1;
	}
	else if(port == GPIOC){
		return 2;
	}
	else if(port == GPIOD){
		return 3;
	}
	else if(port == GPIOE){
		return 4;
	}
	else if(port == GPIOF){
		return 5;
	}
	else if(port == GPIOG){
		return 6;
	}
	else {
		return 0;
	}
}

static void sys_cfg_set_port_for_exti(GPIO_TypeDef * port, uint32_t pin)
{
	uint32_t regIndex = sys_cfg_exti_cr_reg_index_from_pin(pin);
	uint32_t startIndex = sys_cfg_exti_cr_start_bit_from_pin(pin);
	uint32_t portValue = sys_cfg_exti_value_for_port(port);

	// erase target bits
	SYSCFG->EXTICR[regIndex] &= ~(1U<<startIndex);
	SYSCFG->EXTICR[regIndex] &= ~(1U<<(startIndex+1U));
	SYSCFG->EXTICR[regIndex] &= ~(1U<<(startIndex+2U));
	SYSCFG->EXTICR[regIndex] &= ~(1U<<(startIndex+3U));

	// select port for exti
	SYSCFG->EXTICR[regIndex] |= (portValue<<startIndex);
}

static void sys_cfg_erase_port_for_exti(GPIO_TypeDef * port, uint32_t pin)
{
	uint32_t regIndex = sys_cfg_exti_cr_reg_index_from_pin(pin);
	uint32_t startIndex = sys_cfg_exti_cr_start_bit_from_pin(pin);

	// erase target bits
	SYSCFG->EXTICR[regIndex] &= ~(1U<<startIndex);
	SYSCFG->EXTICR[regIndex] &= ~(1U<<(startIndex+1U));
	SYSCFG->EXTICR[regIndex] &= ~(1U<<(startIndex+2U));
	SYSCFG->EXTICR[regIndex] &= ~(1U<<(startIndex+3U));
}

static IRQn_Type get_irq_vect_for_pin(uint32_t pin)
{
	if(pin == 0){
		return EXTI0_IRQn;
	}
	else if(pin == 1){
		return EXTI1_IRQn;
	}
	else if(pin == 2){
		return EXTI2_IRQn;
	}
	else if(pin == 3){
		return EXTI3_IRQn;
	}
	else if(pin == 4){
		return EXTI4_IRQn;
	}
	else if(pin > 4 && pin < 10){
		return EXTI9_5_IRQn;
	}
	else
	{
		return EXTI15_10_IRQn;
	}
}

static GPIO_TypeDef* sys_cfg_port_from_exti_value(uint32_t value)
{
	switch(value)
	{
	case 0:
		return GPIOA;
	case 1:
		return GPIOB;
	case 2:
		return GPIOC;
	case 3:
		return GPIOD;
	case 4:
		return GPIOE;
	case 5:
		return GPIOF;
	case 6:
		return GPIOG;
	default:
		return GPIOA;
	}
}

static void construct_obj_and_trigger_callback(uint32_t pin)
{
	ExternalInterruptObject obj;
	obj.extIntPin = pin;

	uint32_t regIndex = sys_cfg_exti_cr_reg_index_from_pin(pin);
	uint32_t startIndex = sys_cfg_exti_cr_start_bit_from_pin(pin);

	uint32_t regValue = SYSCFG->EXTICR[regIndex];
	uint32_t regMask = 0;
	regMask |= (1U<<startIndex);
	regMask |= (1U<<(startIndex+1U));
	regMask |= (1U<<(startIndex+2U));
	regMask |= (1U<<(startIndex+3U));

	regValue &= regMask;
	regValue >>= startIndex;

	obj.extIntPort = sys_cfg_port_from_exti_value(regValue);
	obj.triggerType = (EXTI->FTSR & (1U<<pin)) ? EXT_INT_TRIGGER_TYPE_FALLING_EDGE : EXT_INT_TRIGGER_TYPE_RISING_EDGE;

	// find registered callback
	for(uint32_t i = 0; i < 16; i++)
	{
		if(extiRegister[i].pin == obj.extIntPin && extiRegister[i].port == obj.extIntPort)
		{
			extiRegister[i].exti_callback(extiRegister[i].opt_param);
			break;
		}
	}
}




