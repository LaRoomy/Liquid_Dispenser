#ifndef SHAPE_H
#define	SHAPE_H

#include <stdint.h>

typedef struct
{
	int x;
	int y;

}POINT, *PPOINT;

typedef struct
{
	int x;
	int y;
	int cx;
	int cy;

}RECT, *PRECT;

typedef struct
{
	int cx;
	int cy;
}SIZE, *PSIZE;

#endif // SHAPE_H
