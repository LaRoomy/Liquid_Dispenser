#include <stm32f4xx.h>
#include <delay.h>
#include "ili9341.h"
#include "spi.h"
#include "common.h"

// TODO: return value + validate input self
// TODO: deinit function

SPIObject displaySpiObj;

static void ILI9341_select()
{
	cs_enable(&displaySpiObj);
}

static void ILI9341_unselect()
{
	cs_disable(&displaySpiObj);
}

static void ILI9341_reset(ILI9341DisplayObject * self)
{
	self->resetPort->ODR &= ~(1U<<(self->resetPinNumber));
	delay_ms(5, self->sys_frq);
    self->resetPort->ODR |= (1U<<(self->resetPinNumber));
}

static void ILI9341_write_command(ILI9341DisplayObject * self, uint8_t cmd)
{
	// set dc pin to command (low)
    self->dcPort->ODR &= ~(1U<<(self->dcPinNumber));
    spi_transmit(&displaySpiObj, &cmd, sizeof(cmd));
}

static void ILI9341_write_data(ILI9341DisplayObject * self, uint8_t* buff, size_t buff_size)
{
	// set dc pin to data (high)
	self->dcPort->ODR |= (1U<<(self->dcPinNumber));
	spi_transmit(&displaySpiObj, buff, buff_size);
}

static void ILI9341_set_address_window(ILI9341DisplayObject * self, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    // column address set
    ILI9341_write_command(self, 0x2A); // CASET
    {
        uint8_t data[] = { (x0 >> 8) & 0xFF, x0 & 0xFF, (x1 >> 8) & 0xFF, x1 & 0xFF };
        ILI9341_write_data(self, data, sizeof(data));
    }

    // row address set
    ILI9341_write_command(self, 0x2B); // RASET
    {
        uint8_t data[] = { (y0 >> 8) & 0xFF, y0 & 0xFF, (y1 >> 8) & 0xFF, y1 & 0xFF };
        ILI9341_write_data(self, data, sizeof(data));
    }

    // write to RAM
    ILI9341_write_command(self, 0x2C); // RAMWR
}

void ILI9341_init(ILI9341DisplayObject * self)
{
	ILI9341_unselect();

	// init spi
	displaySpiObj.spiNumber = self->spiNumber;
	displaySpiObj.chipSelectPinNumber = self->csPinNumber;
	displaySpiObj.chipSelectPinPort = self->csPort;
	displaySpiObj.baudRatePrescaler = 2;
	spi_init(&displaySpiObj);

	// config pins for ds and reset
	// ds:
	uint32_t firstModerBit = 2U*self->dcPinNumber;
	self->dcPort->MODER |= (1U<<firstModerBit);
	self->dcPort->MODER &= ~(1U<<(firstModerBit+1U));
	// reset:
	firstModerBit = 2U*self->resetPinNumber;
	self->resetPort->MODER |= (1U<<firstModerBit);
	self->resetPort->MODER &= ~(1U<<(firstModerBit+1U));

	// NOTE: clock access to the port(s) is not enabled automatically!

    ILI9341_select();
    ILI9341_reset(self);

    // command list is based on https://github.com/martnak/STM32-ILI9341

    // SOFTWARE RESET
    ILI9341_write_command(self, 0x01);
    delay_ms(1000, self->sys_frq);
        
    // POWER CONTROL A
    ILI9341_write_command(self, 0xCB);
    {
        uint8_t data[] = { 0x39, 0x2C, 0x00, 0x34, 0x02 };
        ILI9341_write_data(self, data, sizeof(data));
    }

    // POWER CONTROL B
    ILI9341_write_command(self, 0xCF);
    {
        uint8_t data[] = { 0x00, 0xC1, 0x30 };
        ILI9341_write_data(self, data, sizeof(data));
    }

    // DRIVER TIMING CONTROL A
    ILI9341_write_command(self, 0xE8);
    {
        uint8_t data[] = { 0x85, 0x00, 0x78 };
        ILI9341_write_data(self, data, sizeof(data));
    }

    // DRIVER TIMING CONTROL B
    ILI9341_write_command(self, 0xEA);
    {
        uint8_t data[] = { 0x00, 0x00 };
        ILI9341_write_data(self, data, sizeof(data));
    }

    // POWER ON SEQUENCE CONTROL
    ILI9341_write_command(self, 0xED);
    {
        uint8_t data[] = { 0x64, 0x03, 0x12, 0x81 };
        ILI9341_write_data(self, data, sizeof(data));
    }

    // PUMP RATIO CONTROL
    ILI9341_write_command(self, 0xF7);
    {
        uint8_t data[] = { 0x20 };
        ILI9341_write_data(self, data, sizeof(data));
    }

    // POWER CONTROL,VRH[5:0]
    ILI9341_write_command(self, 0xC0);
    {
        uint8_t data[] = { 0x23 };
        ILI9341_write_data(self, data, sizeof(data));
    }

    // POWER CONTROL,SAP[2:0];BT[3:0]
    ILI9341_write_command(self, 0xC1);
    {
        uint8_t data[] = { 0x10 };
        ILI9341_write_data(self, data, sizeof(data));
    }

    // VCM CONTROL
    ILI9341_write_command(self, 0xC5);
    {
        uint8_t data[] = { 0x3E, 0x28 };
        ILI9341_write_data(self, data, sizeof(data));
    }

    // VCM CONTROL 2
    ILI9341_write_command(self, 0xC7);
    {
        uint8_t data[] = { 0x86 };
        ILI9341_write_data(self, data, sizeof(data));
    }

    // MEMORY ACCESS CONTROL
    ILI9341_write_command(self, 0x36);
    {
        uint8_t data[] = { 0x48 };
        ILI9341_write_data(self, data, sizeof(data));
    }

    // PIXEL FORMAT
    ILI9341_write_command(self, 0x3A);
    {
        uint8_t data[] = { 0x55 };
        ILI9341_write_data(self, data, sizeof(data));
    }

    // FRAME RATIO CONTROL, STANDARD RGB COLOR
    ILI9341_write_command(self, 0xB1);
    {
        uint8_t data[] = { 0x00, 0x18 };
        ILI9341_write_data(self, data, sizeof(data));
    }

    // DISPLAY FUNCTION CONTROL
    ILI9341_write_command(self, 0xB6);
    {
        uint8_t data[] = { 0x08, 0x82, 0x27 };
        ILI9341_write_data(self, data, sizeof(data));
    }

    // 3GAMMA FUNCTION DISABLE
    ILI9341_write_command(self, 0xF2);
    {
        uint8_t data[] = { 0x00 };
        ILI9341_write_data(self, data, sizeof(data));
    }

    // GAMMA CURVE SELECTED
    ILI9341_write_command(self, 0x26);
    {
        uint8_t data[] = { 0x01 };
        ILI9341_write_data(self, data, sizeof(data));
    }

    // POSITIVE GAMMA CORRECTION
    ILI9341_write_command(self, 0xE0);
    {
        uint8_t data[] = { 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
                           0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00 };
        ILI9341_write_data(self, data, sizeof(data));
    }

    // NEGATIVE GAMMA CORRECTION
    ILI9341_write_command(self, 0xE1);
    {
        uint8_t data[] = { 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
                           0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F };
        ILI9341_write_data(self, data, sizeof(data));
    }

    // EXIT SLEEP
    ILI9341_write_command(self, 0x11);

    delay_ms(120, self->sys_frq);

    // TURN ON DISPLAY
    ILI9341_write_command(self, 0x29);

    // MADCTL
    ILI9341_write_command(self, 0x36);
    {
        uint8_t data[] = { ILI9341_ROTATION };
        ILI9341_write_data(self, data, sizeof(data));
    }

    ILI9341_unselect();
}

void ILI9341_draw_pixel(ILI9341DisplayObject * self, uint16_t x, uint16_t y, uint16_t color)
{
    if((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT))
        return;

    ILI9341_select();

    ILI9341_set_address_window(self, x, y, x+1, y+1);
    uint8_t data[] = { color >> 8, color & 0xFF };
    ILI9341_write_data(self, data, sizeof(data));

    ILI9341_unselect();
}

static void ILI9341_write_char(ILI9341DisplayObject * self, uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor)
{
    uint32_t i, b, j;

    ILI9341_set_address_window(self, x, y, x+font.width-1, y+font.height-1);

    for(i = 0; i < font.height; i++) {
        b = font.data[(ch - 32) * font.height + i];
        for(j = 0; j < font.width; j++) {
            if((b << j) & 0x8000)  {
                uint8_t data[] = { color >> 8, color & 0xFF };
                ILI9341_write_data(self, data, sizeof(data));
            } else {
                uint8_t data[] = { bgcolor >> 8, bgcolor & 0xFF };
                ILI9341_write_data(self, data, sizeof(data));
            }
        }
    }
}

void ILI9341_write_string(ILI9341DisplayObject * self, uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor)
{
    ILI9341_select();

    while(*str) {
        if(x + font.width >= ILI9341_WIDTH) {
            x = 0;
            y += font.height;
            if(y + font.height >= ILI9341_HEIGHT) {
                break;
            }

            if(*str == ' ') {
                // skip spaces in the beginning of the new line
                str++;
                continue;
            }
        }

        ILI9341_write_char(self, x, y, *str, font, color, bgcolor);
        x += font.width;
        str++;
    }

    ILI9341_unselect();
}

void ILI9341_fill_rectangle(ILI9341DisplayObject * self, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    // clipping
    if((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT)) return;
    if((x + w - 1) >= ILI9341_WIDTH) w = ILI9341_WIDTH - x;
    if((y + h - 1) >= ILI9341_HEIGHT) h = ILI9341_HEIGHT - y;

    ILI9341_select();
    ILI9341_set_address_window(self, x, y, x+w-1, y+h-1);

    uint8_t data[] = { color >> 8, color & 0xFF };

    //HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port, ILI9341_DC_Pin, GPIO_PIN_SET);
    self->dcPort->ODR |= (1U<<(self->dcPinNumber));

    for(y = h; y > 0; y--) {
        for(x = w; x > 0; x--) {
            //HAL_SPI_Transmit(&ILI9341_SPI_PORT, data, sizeof(data), HAL_MAX_DELAY);
        	spi_transmit(&displaySpiObj, data, sizeof(data));
        }
    }

    ILI9341_unselect();
}

void ILI9341_fill_screen(ILI9341DisplayObject * self, uint16_t color) {
    ILI9341_fill_rectangle(self, 0, 0, ILI9341_WIDTH, ILI9341_HEIGHT, color);
}

void ILI9341_draw_image(ILI9341DisplayObject * self, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data)
{
    if((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT)) return;
    if((x + w - 1) >= ILI9341_WIDTH) return;
    if((y + h - 1) >= ILI9341_HEIGHT) return;

    ILI9341_select();
    ILI9341_set_address_window(self, x, y, x+w-1, y+h-1);
    ILI9341_write_data(self, (uint8_t*)data, sizeof(uint16_t)*w*h);
    ILI9341_unselect();
}

void ILI9341_invert_colors(ILI9341DisplayObject * self, BOOL invert)
{
    ILI9341_select();
    ILI9341_write_command(self, invert ? 0x21 /* INVON */ : 0x20 /* INVOFF */);
    ILI9341_unselect();
}

