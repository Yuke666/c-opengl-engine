#ifndef UTILS_DEF
#define UTILS_DEF

#include "math.h"
#include "game.h"

typedef struct {
    Vec2 pos;
    Vec2 coord;
} Vertex22;

typedef struct {
	int width;
	int height;
	u32 texture;
	u8 nFramesX;
	u8 nFramesY;
} Image;

void Utils_LoadImage(Image *tex, const char *path, int filter, int channels);
void Utils_Vertex22Rect(Vertex22 *out, Rect2D screenRect, Rect2D imgRect);

#endif