#include "stm32f4xx.h"
#include "ili9341_touch.h"
#include "spi.h"
#include "common.h"

#define READ_X 0xD0
#define READ_Y 0x90

SPIObject touchSpiObj;

static void ILI9341_touch_unselect();

int ILI9341_touch_init(ILI9341TouchObject * self)
{
	ILI9341_touch_unselect();

	// init spi
	touchSpiObj.spiNumber = self->spiNumber;
	touchSpiObj.chipSelectPinNumber = self->csPinNumber;
	touchSpiObj.chipSelectPinPort = self->csGPIOPort;
	touchSpiObj.baudRatePrescaler = 32;

	if(spi_init(&touchSpiObj) != SPI_OK)
	{
		// return invalid arg
		return -22;
	}
	// NOTE: clock access to the port(s) is not enabled automatically!
	// config irq pin as input
	uint32_t moderNr = self->irqPinNumber*2U;
	self->irqGPIOPort->MODER &= ~(1U<<(moderNr));
	self->irqGPIOPort->MODER &= ~(1U<<(moderNr+1U));

	return 0;
}

static void ILI9341_touch_select()
{
	cs_enable(&touchSpiObj);
}

static void ILI9341_touch_unselect()
{
	cs_disable(&touchSpiObj);
}

BOOL ILI9341_touch_pressed(ILI9341TouchObject * self)
{
	return ((self->irqGPIOPort->IDR & (1U<<(self->irqPinNumber))) == 0 ? TRUE : FALSE);
}

int ILI9341_touch_get_coordinates(ILI9341TouchObject * self, uint16_t* x, uint16_t* y, uint32_t rotation)
{
    uint8_t cmd_read_x[] = { READ_X };
    uint8_t cmd_read_y[] = { READ_Y };

    ILI9341_touch_select();

    uint32_t avg_x = 0;
    uint32_t avg_y = 0;
    uint8_t nsamples = 0;

    for(uint8_t i = 0; i < 16; i++)
    {
        if(ILI9341_touch_pressed(self) == FALSE)
        {
            break;
        }
        nsamples++;

        uint8_t y_raw[2];
        spi_transmit(&touchSpiObj, cmd_read_y, sizeof(cmd_read_y));
        spi_receive(&touchSpiObj, y_raw, sizeof(y_raw));

        uint8_t x_raw[2];
        spi_transmit(&touchSpiObj, cmd_read_x, sizeof(cmd_read_x));
        spi_receive(&touchSpiObj, x_raw, sizeof(x_raw));

        avg_x += (((uint16_t)x_raw[0]) << 8) | ((uint16_t)x_raw[1]);
        avg_y += (((uint16_t)y_raw[0]) << 8) | ((uint16_t)y_raw[1]);
    }

    ILI9341_touch_unselect();

    if(nsamples < 16)
        return -1;

    uint32_t raw_x = (avg_x / 16);
    if(raw_x < ILI9341_TOUCH_MIN_RAW_X) raw_x = ILI9341_TOUCH_MIN_RAW_X;
    if(raw_x > ILI9341_TOUCH_MAX_RAW_X) raw_x = ILI9341_TOUCH_MAX_RAW_X;

    uint32_t raw_y = (avg_y / 16);
    if(raw_y < ILI9341_TOUCH_MIN_RAW_Y) raw_y = ILI9341_TOUCH_MIN_RAW_Y;
    if(raw_y > ILI9341_TOUCH_MAX_RAW_Y) raw_y = ILI9341_TOUCH_MAX_RAW_Y;

    uint32_t x_holder = (raw_x - ILI9341_TOUCH_MIN_RAW_X) * ILI9341_TOUCH_SCALE_X / (ILI9341_TOUCH_MAX_RAW_X - ILI9341_TOUCH_MIN_RAW_X);
    uint32_t y_holder = (raw_y - ILI9341_TOUCH_MIN_RAW_Y) * ILI9341_TOUCH_SCALE_Y / (ILI9341_TOUCH_MAX_RAW_Y - ILI9341_TOUCH_MIN_RAW_Y);

    if(rotation == ILI9341_TOUCH_ROTATE_LEFT)
    {
    	*x = (uint16_t)(320 - y_holder);
    	*y = (uint16_t)(240 - x_holder);
    }
    else if(rotation == ILI9341_TOUCH_ROTATE_RIGHT)
    {
    	*x = (uint16_t)(y_holder);
    	*y = (uint16_t)(x_holder);
    }
    else if(rotation == ILI9341_TOUCH_ROTATE_UPSIDEDOWN)
    {
    	*x = (uint16_t)(240 - x_holder);
    	*y = (uint16_t)(y_holder);
    }
    else
    {
    	*x = (uint16_t)x_holder;
    	*y = (uint16_t)(320 - y_holder);
    }
    return 0;
}
