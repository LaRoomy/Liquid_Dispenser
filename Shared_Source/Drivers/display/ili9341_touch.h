#ifndef __ILI9341_TOUCH_H__
#define __ILI9341_TOUCH_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "common.h"
#include "stm32f4xx.h"

typedef struct
{
	uint32_t spiNumber;
	GPIO_TypeDef * csGPIOPort;
	uint32_t csPinNumber;
	GPIO_TypeDef * irqGPIOPort;
	uint32_t irqPinNumber;

}ILI9341TouchObject;

// change depending on screen orientation

#define ILI9341_TOUCH_SCALE_X 240
#define ILI9341_TOUCH_SCALE_Y 320

#define ILI9341_TOUCH_MIN_RAW_X 1500
#define ILI9341_TOUCH_MAX_RAW_X 31000
#define ILI9341_TOUCH_MIN_RAW_Y 3276
#define ILI9341_TOUCH_MAX_RAW_Y 30110


/*
#define ILI9341_TOUCH_SCALE_X 320
#define ILI9341_TOUCH_SCALE_Y 240

#define ILI9341_TOUCH_MIN_RAW_Y 1500
#define ILI9341_TOUCH_MAX_RAW_Y 31000
#define ILI9341_TOUCH_MIN_RAW_X 3276
#define ILI9341_TOUCH_MAX_RAW_X 30110
*/

#define		ILI9341_TOUCH_DEFAULT_ROTATION		0
#define		ILI9341_TOUCH_ROTATE_RIGHT			1
#define		ILI9341_TOUCH_ROTATE_LEFT			2
#define		ILI9341_TOUCH_ROTATE_UPSIDEDOWN		3


BOOL ILI9341_touch_pressed(ILI9341TouchObject * self);
int ILI9341_touch_get_coordinates(ILI9341TouchObject * self, uint16_t* x, uint16_t* y, uint32_t rotation);
int ILI9341_touch_init(ILI9341TouchObject * self);

#ifdef __cplusplus
 } // extern "C"
#endif

#endif // __ILI9341_TOUCH_H__
