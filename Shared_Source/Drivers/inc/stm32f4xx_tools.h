#ifndef STM32F4XX_TOOLS_H_
#define STM32F4XX_TOOLS_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stm32f4xx.h>
#include <common.h>

#define		GPIO_MODE_INPUT		0U
#define		GPIO_MODE_OUTPUT	1U
#define		GPIO_MODE_ALTERNATE	2U
#define		GPIO_MODE_ANALOG	3U

#define		GPIO_MODE_RESET_STATE	GPIO_MODE_INPUT

#define		GPIO_OUTPUT_TYPE_PUSH_PULL		0U
#define		GPIO_OUTPUT_TYPE_OPEN_DRAIN		1U

#define		GPIO_CONFIG_OK							0
#define		GPIO_CONFIG_ERROR_INVALID_ARGUMENT		(-22);

int rcc_enable_clock_access_for_port(GPIO_TypeDef * port);
int gpio_config_port_pin(GPIO_TypeDef * port, uint32_t pin, uint32_t mode);
int gpio_set_alternate_function_to_pin(GPIO_TypeDef * port, uint32_t pin, uint32_t afNumber);
int gpio_set_output_type(GPIO_TypeDef * port, uint32_t pin, uint32_t type);
int gpio_control_pin(GPIO_TypeDef * port, uint32_t pin, uint32_t state);
uint32_t gpio_read_pin(GPIO_TypeDef * port, uint32_t pin);

BOOL is_valid_port(GPIO_TypeDef * port);
BOOL is_valid_pin(uint32_t pinNumber);
BOOL is_valid_port_pin(GPIO_TypeDef * port, uint32_t pin);

#ifdef __cplusplus
 } // extern "C"
#endif

#endif /* STM32F4XX_TOOLS_H_ */
