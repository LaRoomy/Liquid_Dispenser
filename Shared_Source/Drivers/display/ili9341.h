#ifndef __ILI9341_H__
#define __ILI9341_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "fonts.h"
#include "common.h"
#include <stddef.h>
#include <stm32f4xx.h>

typedef struct
{
	uint32_t spiNumber;
	GPIO_TypeDef * csPort;
	uint32_t csPinNumber;
	GPIO_TypeDef * dcPort;
	uint32_t dcPinNumber;
	GPIO_TypeDef * resetPort;
	uint32_t resetPinNumber;
	uint32_t sys_frq;

}ILI9341DisplayObject;

#define ILI9341_MADCTL_MY  0x80
#define ILI9341_MADCTL_MX  0x40
#define ILI9341_MADCTL_MV  0x20
#define ILI9341_MADCTL_ML  0x10
#define ILI9341_MADCTL_RGB 0x00
#define ILI9341_MADCTL_BGR 0x08
#define ILI9341_MADCTL_MH  0x04

// default orientation
/*
#define ILI9341_WIDTH  240
#define ILI9341_HEIGHT 320
#define ILI9341_ROTATION (ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR)
*/

// rotate right
/*
#define ILI9341_WIDTH  320
#define ILI9341_HEIGHT 240
#define ILI9341_ROTATION (ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR)
*/

// rotate left
#define ILI9341_WIDTH  320
#define ILI9341_HEIGHT 240
#define ILI9341_ROTATION (ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR)

// upside down
/*
#define ILI9341_WIDTH  240
#define ILI9341_HEIGHT 320
#define ILI9341_ROTATION (ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR)
*/

/****************************/

// Color definitions
#define	ILI9341_BLACK   0x0000
#define	ILI9341_BLUE    0x001F
#define	ILI9341_RED     0xF800
#define	ILI9341_GREEN   0x07E0
#define ILI9341_CYAN    0x07FF
#define ILI9341_MAGENTA 0xF81F
#define ILI9341_YELLOW  0xFFE0
#define ILI9341_WHITE   0xFFFF
#define	ILI9341_ORANGE	0xDBA0
#define	ILI9341_MINT	0xAFF9
#define	ILI9341_PURPLE	0xE279
#define ILI9341_PINK	0xE8D4
#define ILI9341_COLOR565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))


void ILI9341_init(ILI9341DisplayObject * self);
void ILI9341_draw_pixel(ILI9341DisplayObject * self, uint16_t x, uint16_t y, uint16_t color);
void ILI9341_write_string(ILI9341DisplayObject * self, uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor);
void ILI9341_fill_rectangle(ILI9341DisplayObject * self, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ILI9341_fill_screen(ILI9341DisplayObject * self, uint16_t color);
void ILI9341_draw_image(ILI9341DisplayObject * self, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data);
void ILI9341_invert_colors(ILI9341DisplayObject * self, BOOL invert);

#ifdef __cplusplus
 } // extern "C"
#endif

#endif // __ILI9341_H__
