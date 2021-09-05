#ifndef TEXT_DEF
#define TEXT_DEF

#include "math.h"

void Text_Init(void);
void Text_Draw(int x, int y, int hSpacing, int vSpacing, int maxWidth, const char *text);
void Text_Close(void);
void Text_DrawRect(unsigned int texture, Rect2D screenRect, Rect2D imgRect);

#endif