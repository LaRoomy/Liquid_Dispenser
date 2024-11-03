#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>

typedef struct
{
	const uint16_t * imageData;
	int pos_x;
	int pos_y;
	int width;
	int heigth;

}IMAGE, *PIMAGE;

#endif // IMAGE_H
