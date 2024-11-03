#include "DisplayImplementation.hpp"
#include <string.h>

// TODO: remove !!!
//ILI9341DisplayObject iliDisplayObj =
//{
//	.spiNumber = 2,
//	.csPort = GPIOB,
//	.csPinNumber = 1,
//	.dcPort = GPIOB,
//	.dcPinNumber = 2,
//	.resetPort = GPIOB,
//	.resetPinNumber = 12,
//	.sys_frq = MCU_CLOCK
//};

bool CDisplayController::displayControllerInstanceCreated = false;
CDisplayController * CDisplayController::displayControllerInstance = nullptr;

CDisplayController * CDisplayController::GetInstance()
{
	if(displayControllerInstanceCreated == false)
	{
		displayControllerInstance = new CDisplayController();
		if(displayControllerInstance != nullptr)
		{
			displayControllerInstanceCreated = true;
		}
	}
	return displayControllerInstance;
}

CDisplayController::CDisplayController()
{
	this->iliDisplayObj.spiNumber = 2;
	this->iliDisplayObj.csPort = GPIOB;
	this->iliDisplayObj.csPinNumber = 1;
	this->iliDisplayObj.dcPort = GPIOB;
	this->iliDisplayObj.dcPinNumber = 2;
	this->iliDisplayObj.resetPort = GPIOB;
	this->iliDisplayObj.resetPinNumber = 12;
	this->iliDisplayObj.sys_frq = MCU_CLOCK;

	ILI9341_init(&this->iliDisplayObj);
	ILI9341_fill_screen(&this->iliDisplayObj, ILI9341_BLACK);
}

uint32_t CDisplayController::getScreenWidth()
{
	return ILI9341_WIDTH;
}

uint32_t CDisplayController::getScreenHeight()
{
	return ILI9341_HEIGHT;
}

void CDisplayController::getTextSize(const char * text, Font font, PSIZE textSize_out)
{
	if(textSize_out != nullptr && text != nullptr)
	{
		int textLen = strlen(text);

		if(font == Font::Large)
		{
			textSize_out->cy = 26;
			textSize_out->cx = 16 * textLen;
		}
		else if(font == Font::Medium)
		{
			textSize_out->cy = 18;
			textSize_out->cx = 11 * textLen;
		}
		else if(font == Font::Small)
		{
			textSize_out->cy = 10;
			textSize_out->cx = 7 * textLen;
		}
	}
}

void CDisplayController::drawImage(PIMAGE pImage)
{
	if(pImage != nullptr)
	{
		ILI9341_draw_image(&this->iliDisplayObj,pImage->pos_x, pImage->pos_y, pImage->width, pImage->heigth, (const uint16_t*)pImage->imageData);
	}
}

void CDisplayController::drawText(PPOINT pCoordinates, const char * text, Font font, const COLOR& foregroundColor, const COLOR& backgroundColor)
{
	if(pCoordinates != nullptr && text != nullptr)
	{
		ILI9341_write_string(
			&iliDisplayObj,
			pCoordinates->x,
			pCoordinates->y,
			text,
			*(this->getFontData(font)),
			ILI9341_COLOR565(foregroundColor.red, foregroundColor.green, foregroundColor.blue),
			ILI9341_COLOR565(backgroundColor.red, backgroundColor.green, backgroundColor.blue)
		);
	}
}

void CDisplayController::drawPixel(PPOINT pPoint, const COLOR& color)
{
	if(pPoint != nullptr)
	{
		ILI9341_draw_pixel(&iliDisplayObj, pPoint->x, pPoint->y, ILI9341_COLOR565(color.red, color.green, color.blue));
	}
}

void CDisplayController::fillRect(PRECT pRect, const COLOR& color)
{
	if(pRect != nullptr)
	{
		ILI9341_fill_rectangle(&this->iliDisplayObj, pRect->x, pRect->y, pRect->cx, pRect->cy, ILI9341_COLOR565(color.red, color.green, color.blue));
	}
}

void CDisplayController::drawRect(PRECT pRect, const COLOR& color)
{
	if(pRect != nullptr)
	{
		auto _col = (uint16_t)ILI9341_COLOR565(color.red, color.green, color.blue);
		POINT top_left = { pRect->x, pRect->y };
		POINT top_right = { pRect->x + pRect->cx, pRect->y };
		POINT bottom_right = { pRect->x + pRect->cx, pRect->y + pRect->cy };
		POINT bottom_left = { pRect->x, pRect->y + pRect->cy };

		for(uint16_t x = (uint16_t)top_left.x; x < (uint16_t)top_right.x; x++)
		{
			ILI9341_draw_pixel(&this->iliDisplayObj, x, top_left.y, _col);
		}
		for(uint16_t y = (uint16_t)top_right.y; y < (uint16_t)bottom_right.y; y++)
		{
			ILI9341_draw_pixel(&this->iliDisplayObj, top_right.x, y, _col);
		}
		for(uint16_t x = (uint16_t)bottom_right.x; x > (uint16_t)bottom_left.x; x--)
		{
			ILI9341_draw_pixel(&this->iliDisplayObj, x, bottom_right.y, _col);
		}
		for(uint16_t y = (uint16_t)bottom_left.y; y > (uint16_t)top_left.y; y--)
		{
			ILI9341_draw_pixel(&iliDisplayObj, bottom_left.x, y, _col);
		}
	}
}

void CDisplayController::fillScreen(const COLOR& color)
{
	ILI9341_fill_screen(&iliDisplayObj, ILI9341_COLOR565(color.red, color.green, color.blue));
}

FontDef * CDisplayController::getFontData(Font font)
{
	switch(font)
	{
	case Font::Large:
		return &Font_16x26;
	case Font::Medium:
		return &Font_11x18;
	case Font::Small:
		return &Font_7x10;
	default:
		return nullptr;
	}
}

// *************************************

IDisplayController * GetDisplayControllerInstance()
{
	return CDisplayController::GetInstance();
}
