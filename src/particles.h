#ifndef PARTICLES_DEF
#define PARTICLES_DEF


#include "math.h"
#include "game.h"
#include "utils.h"

typedef struct {
	float 	createTime;
	float 	lifeTime;
	float  	distFromEye;
	Vec3 	pos;
	Vec2 	size;
	Vec4 	color;
} Particle;

void Particles_DrawBillboard(Image img, Vec3 pos, Vec2 size, Vec4 color, Rect2D imgRect);
void Particles_DrawParticles(Image img, Particle *particles, int nParticles, float animSpeed, Vec3 camForward, Vec3 camPos, int depthTexture);
void Particles_Init(void);
void Particles_Close(void);

#endif