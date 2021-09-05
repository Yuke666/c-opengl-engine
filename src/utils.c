#include <GL/glew.h>
#include "utils.h"
#include "memory.h"
#include "deflate.h"

void Utils_LoadImage(Image *tex, const char *path, int filter, int channels){

	FILE *fp = fopen(path, "rb");

	int w, h;
	fread(&w, 1, sizeof(int), fp);
	fread(&h, 1, sizeof(int), fp);

    glGenTextures(1, &tex->texture);
    glBindTexture(GL_TEXTURE_2D, tex->texture);

    tex->width = w;
    tex->height = h;
    tex->nFramesX = 1;
    tex->nFramesY = 1;

	u8 *data = (u8 *)Memory_StackAlloc(MAIN_STACK, w * h * channels);
	Deflate_Read(fp, data, w * h * channels);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, channels == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, data);
	Memory_StackPop(MAIN_STACK, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);

	fclose(fp);
}

void Utils_Vertex22Rect(Vertex22 *out, Rect2D screenRect, Rect2D imgRect){
    
    out[0].pos = (Vec2){ screenRect.x, screenRect.y};
    out[0].coord = (Vec2){ imgRect.x, imgRect.y+imgRect.h};
    out[1].pos = (Vec2){ screenRect.x, screenRect.y+screenRect.h};
    out[1].coord = (Vec2){ imgRect.x, imgRect.y};
    out[2].pos = (Vec2){ screenRect.x+screenRect.w, screenRect.y+screenRect.h};
    out[2].coord = (Vec2){ imgRect.x+imgRect.w, imgRect.y};
    out[3].pos = (Vec2){ screenRect.x+screenRect.w, screenRect.y+screenRect.h};
    out[3].coord = (Vec2){ imgRect.x+imgRect.w, imgRect.y};
    out[4].pos = (Vec2){ screenRect.x+screenRect.w, screenRect.y};
    out[4].coord = (Vec2){ imgRect.x+imgRect.w, imgRect.y+imgRect.h};
    out[5].pos = (Vec2){ screenRect.x, screenRect.y};
    out[5].coord = (Vec2){ imgRect.x, imgRect.y+imgRect.h};
}