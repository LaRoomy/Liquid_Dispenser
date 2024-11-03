#ifndef DISPLAYCOLORS_HPP
#define DISPLAYCOLORS_HPP

#include <stdint.h>

typedef struct
{
	uint8_t red;
	uint8_t green;
	uint8_t blue;

}COLOR, *PCOLOR;

class Colors
{
public:
	static const COLOR Red;
	static const COLOR Green;
	static const COLOR Blue;
	static const COLOR White;
	static const COLOR Black;
	static const COLOR Cyan;
	static const COLOR Magenta;
	static const COLOR Yellow;
	static const COLOR Orange;
	static const COLOR Purple;
	static const COLOR Mint;
	static const COLOR Aqua;
	static const COLOR Brown;
	static const COLOR Pink;
	static const COLOR SlateGray;
	static const COLOR YellowGreen;
	static const COLOR GreenYellow;
	static const COLOR Gold;
	static const COLOR Gray;
	static const COLOR Coral;
	static const COLOR SeaGreen;
	static const COLOR DeepPink;
	static const COLOR DarkBlue;
	static const COLOR DarkCyan;
	static const COLOR DarkRed;
	static const COLOR DarkGreen;
	static const COLOR DarkMagenta;
	static const COLOR DarkSlateGray;
	static const COLOR DarkSeaGreen;
	static const COLOR DarkGray;
	static const COLOR LightBlue;
	static const COLOR LightCyan;
	static const COLOR LightGray;
	static const COLOR LightGreen;
	static const COLOR LightRed;
};

#endif // DISPLAYCOLORS_HPP
