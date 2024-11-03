#ifndef TOUCHINTERFACE_HPP
#define TOUCHINTERFACE_HPP

#include <stdint.h>

class ITouchController
{
public:
	virtual ~ITouchController() = default;
	virtual bool isTouchPressed() = 0;
	virtual bool getCoordinates(uint16_t * x, uint16_t * y) = 0;
};

ITouchController * GetTouchControllerInstance();

#endif // TOUCHINTERFACE_HPP
