#include <GL/glew.h>
#include "particles.h"
#include "shaders.h"
#include "renderer.h"
#include "window.h"
#include "memory.h"
#include "shader_files.h"

#define MAX_PARTICLES 1024

static u32 vao, centerVbo, positionVbo, uvVbo, colorVbo, ebo;

static int SortParticles(const void *p1, const void *p2){

	Particle *particle1 = (Particle *)p1;
	Particle *particle2 = (Particle *)p2;

	if(particle1->distFromEye < particle2->distFromEye) return 1;
	if(particle1->distFromEye > particle2->distFromEye) return -1;
	return 0;
} 

void Particles_DrawParticles(Image img, Particle *particles, int nParticles, 
	float animSpeed, Vec3 camForward, Vec3 camPos, int depthTexture){

	nParticles = MIN(MAX_PARTICLES, nParticles);

	glUseProgram(Shaders_GetProgram(PROGRAM_particles));

	glBindVertexArray(vao);

	float currTime = Window_GetTicks();

	int num = 0;

	int k;
	for(k = 0; k < nParticles; k++){

		if(particles[k].createTime <= 0 || currTime - particles[k].createTime > particles[k].lifeTime){
			particles[k].distFromEye = -HUGE_VAL;
			continue;
		}

		// particles[k].distFromEye = Math_Vec3Magnitude(Math_Vec3SubVec3(particles[k].pos, camPos));
		particles[k].distFromEye = -Math_Vec3Dot(camForward, Math_Vec3SubVec3(particles[k].pos, camPos));
		++num;
	}

	qsort(particles, nParticles, sizeof(Particle), SortParticles);

	for(k = 0; k < nParticles; k++){

		if(particles[k].distFromEye < 0)
			continue;

		Vec3 pos = particles[k].pos;
		Vec4 color = particles[k].color;
		Vec2 size = particles[k].size;

		Vec3 centers[] = { pos, pos, pos, pos };
		Vec4 colors[] = { color, color, color, color };

		Vec2 positions[] = {
			{ -size.x/2, -size.y/2 },
			{ size.x/2, -size.y/2 },
			{ size.x/2, size.y/2 },
			{ -size.x/2, size.y/2 },
		};

		int onFrame = (currTime - particles[k].createTime) / animSpeed;

		onFrame %= img.nFramesX * img.nFramesY;

		float tsx = 1.0 / img.nFramesX;
		float tsy = 1.0 / img.nFramesX;

		float tx = (onFrame % img.nFramesX) * tsx;
		float ty = (onFrame / img.nFramesX) * tsy;

		Vec2 uvs[] = {
			{ tx, ty },
			{ tx + tsx, ty },
			{ tx + tsx, ty + tsy },
			{ tx, ty + tsy },
		};

		glBindBuffer(GL_ARRAY_BUFFER, centerVbo);
		glBufferSubData(GL_ARRAY_BUFFER, k * sizeof(Vec3) * 4, sizeof(Vec3) * 4, centers);

		glBindBuffer(GL_ARRAY_BUFFER, colorVbo);
		glBufferSubData(GL_ARRAY_BUFFER, k * sizeof(Vec4) * 4, sizeof(Vec4) * 4, colors);

		glBindBuffer(GL_ARRAY_BUFFER, positionVbo);
		glBufferSubData(GL_ARRAY_BUFFER, k * sizeof(Vec2) * 4, sizeof(Vec2) * 4, positions);

		glBindBuffer(GL_ARRAY_BUFFER, uvVbo);
		glBufferSubData(GL_ARRAY_BUFFER, k * sizeof(Vec2) * 4, sizeof(Vec2) * 4, uvs);
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, img.texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	
	glDrawElements(GL_TRIANGLES, num * 6, GL_UNSIGNED_SHORT, NULL);

	glBindVertexArray(0);
}

void Particles_DrawBillboard(Image img, Vec3 pos, Vec2 size, Vec4 color, Rect2D imgRect){

	glUseProgram(Shaders_GetProgram(PROGRAM_particles));

	glBindVertexArray(vao);

	Vec3 centers[] = { pos, pos, pos, pos, pos, pos };
	Vec4 colors[] = { color, color, color, color };

	Vec2 positions[] = {
		{ -size.x/2, -size.y/2 },
		{ size.x/2, -size.y/2 },
		{ size.x/2, size.y/2 },
		{ -size.x/2, size.y/2 },
	};

	Vec2 uvs[] = {
		{ imgRect.x, imgRect.y },
		{ imgRect.x + imgRect.w, imgRect.y },
		{ imgRect.x + imgRect.w, imgRect.y + imgRect.h },
		{ imgRect.x, imgRect.y + imgRect.h },
	};

	glBindBuffer(GL_ARRAY_BUFFER, positionVbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vec2) * 6, positions);

	glBindBuffer(GL_ARRAY_BUFFER, centerVbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vec3) * 4, centers);

	glBindBuffer(GL_ARRAY_BUFFER, colorVbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vec4) * 4, colors);

	glBindBuffer(GL_ARRAY_BUFFER, positionVbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vec2) * 4, positions);

	glBindBuffer(GL_ARRAY_BUFFER, uvVbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vec2) * 6, uvs);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, img.texture);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);

	glBindVertexArray(0);
}

void Particles_Init(void){

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &centerVbo);
	glBindBuffer(GL_ARRAY_BUFFER, centerVbo);

    glEnableVertexAttribArray(CENTER_LOC);
    glVertexAttribPointer(CENTER_LOC, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec3) * MAX_PARTICLES * 4, NULL, GL_STATIC_DRAW);

	glGenBuffers(1, &uvVbo);
	glBindBuffer(GL_ARRAY_BUFFER, uvVbo);

    glEnableVertexAttribArray(UV_LOC);
    glVertexAttribPointer(UV_LOC, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2) * MAX_PARTICLES * 4, NULL, GL_STATIC_DRAW);

	glGenBuffers(1, &colorVbo);
	glBindBuffer(GL_ARRAY_BUFFER, colorVbo);

    glEnableVertexAttribArray(COLOR_LOC);
    glVertexAttribPointer(COLOR_LOC, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec4) * MAX_PARTICLES * 4, NULL, GL_STATIC_DRAW);

	glGenBuffers(1, &positionVbo);
	glBindBuffer(GL_ARRAY_BUFFER, positionVbo);
	
    glEnableVertexAttribArray(POS_LOC);
    glVertexAttribPointer(POS_LOC, 2, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2) * MAX_PARTICLES * 4, NULL, GL_STATIC_DRAW);

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u16) * MAX_PARTICLES * 6, NULL, GL_STATIC_DRAW);

	u16 elements[] = {0, 1, 2, 0, 2, 3};

	int k;
	for(k = 0; k < MAX_PARTICLES; k++){

		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, k * sizeof(u16) * 6, sizeof(u16) * 6, elements);

		int j;
		for(j = 0; j < 6; j++)
			elements[j] += 4;

	}

	glBindVertexArray(0);
}

void Particles_Close(void){

	glDeleteBuffers(1, &positionVbo);
	glDeleteBuffers(1, &centerVbo);
	glDeleteBuffers(1, &uvVbo);
	glDeleteBuffers(1, &colorVbo);
	glDeleteBuffers(1, &ebo);
	glDeleteVertexArrays(1, &vao);
}