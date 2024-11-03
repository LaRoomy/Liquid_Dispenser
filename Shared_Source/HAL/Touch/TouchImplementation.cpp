#include "TouchImplementation.hpp"

bool CTouchController::touchControllerInstanceCreated = false;
CTouchController * CTouchController::touchControllerInstance = nullptr;

CTouchController::CTouchController()
{
	this->ili9341TouchObject.spiNumber = 1U;
	this->ili9341TouchObject.csGPIOPort = GPIOA;
	this->ili9341TouchObject.csPinNumber = 9U;
	this->ili9341TouchObject.irqGPIOPort = GPIOA;
	this->ili9341TouchObject.irqPinNumber = 8U;
	ILI9341_touch_init(&this->ili9341TouchObject);
}

CTouchController * CTouchController::GetInstance()
{
	if(touchControllerInstanceCreated == false)
	{
		touchControllerInstance = new CTouchController();
		if(touchControllerInstance != nullptr)
		{
			touchControllerInstanceCreated = true;
		}
	}
	return touchControllerInstance;
}

bool CTouchController::isTouchPressed()
{
	return (ILI9341_touch_pressed(&this->ili9341TouchObject) == TRUE) ? true : false;
}

bool CTouchController::getCoordinates(uint16_t * x, uint16_t * y)
{
	if(ILI9341_touch_get_coordinates(&this->ili9341TouchObject, x, y, ILI9341_TOUCH_ROTATE_LEFT) == 0)
	{
		return true;
	}
	return false;
}

// ********************************************************************

ITouchController * GetTouchControllerInstance()
{
	return reinterpret_cast<ITouchController*>(CTouchController::GetInstance());
}
