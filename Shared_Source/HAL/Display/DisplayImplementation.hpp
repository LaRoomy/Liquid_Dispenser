#ifndef DISPLAYIMPLEMENTATION_HPP
#define DISPLAYIMPLEMENTATION_HPP

#include <DisplayInterface.hpp>
#include <ili9341.h>

class CDisplayController : public IDisplayController
{
public:
	static CDisplayController * GetInstance();

	uint32_t getScreenWidth() override;
	uint32_t getScreenHeight() override;

	void getTextSize(const char * text, Font font, PSIZE textSize_out) override;

	void drawImage(PIMAGE pImage) override;
	void drawText(PPOINT pCoordinates, const char * text, Font font, const COLOR& foregroundColor, const COLOR& backgroundColor) override;
	void drawPixel(PPOINT pPoint, const COLOR& color) override;
	void fillRect(PRECT pRect, const COLOR& color) override;
	void drawRect(PRECT pRect, const COLOR& color) override;

	void fillScreen(const COLOR& color) override;

private:
	CDisplayController();

	static bool displayControllerInstanceCreated;
	static CDisplayController * displayControllerInstance;

	ILI9341DisplayObject iliDisplayObj;

	FontDef * getFontData(Font font);
};


#endif // DISPLAYIMPLEMENTATION_HPP
