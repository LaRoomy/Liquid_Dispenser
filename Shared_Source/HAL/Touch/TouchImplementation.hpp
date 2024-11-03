#ifndef TOUCHIMPLEMENTATION_HPP
#define TOUCHIMPLEMENTATION_HPP

#include <TouchInterface.hpp>
#include <ili9341_touch.h>

class CTouchController : public ITouchController
{
public:
	static CTouchController * GetInstance();

	bool isTouchPressed() override;
	bool getCoordinates(uint16_t * x, uint16_t * y) override;

private:
	CTouchController();

	static bool touchControllerInstanceCreated;
	static CTouchController * touchControllerInstance;

	ILI9341TouchObject ili9341TouchObject;
};

#endif // TOUCHIMPLEMENTATION_HPP
