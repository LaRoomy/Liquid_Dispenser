#ifndef DISPLAYINTERFACE_HPP
#define DISPLAYINTERFACE_HPP

#include <stdint.h>
#include <shape.h>
#include <image.h>
#include <DisplayColors.hpp>

enum class Font { Large, Medium, Small };

class IDisplayController
{
public:
	virtual ~IDisplayController() = default;

	virtual uint32_t getScreenWidth() = 0;
	virtual uint32_t getScreenHeight() = 0;

	virtual void getTextSize(const char * text, Font font, PSIZE textSize_out) = 0;

	virtual void drawImage(PIMAGE pImage) = 0;
	virtual void drawText(PPOINT pCoordinates, const char * text, Font font, const COLOR& foregroundColor, const COLOR& backgroundColor) = 0;
	virtual void drawPixel(PPOINT pPoint, const COLOR& color) = 0;
	virtual void drawRect(PRECT pRect, const COLOR& color) = 0;

	virtual void fillRect(PRECT pRect, const COLOR& color) = 0;
	virtual void fillScreen(const COLOR& color) = 0;
};

IDisplayController * GetDisplayControllerInstance();

#endif // DISPLAYINTERFACE_HPP
