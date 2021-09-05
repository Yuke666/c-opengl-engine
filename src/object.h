#ifndef OBJECT_DEF
#define OBJECT_DEF

#include "math.h"
#include "model.h"
#include "game.h"

typedef struct {

	u8 transparent;
	u8 occluder;
	
	Skeleton *skeleton;
	void *data;

	float matrix[16];
	float matrixInvTrans[9];
	Model model;

} Object;


#endif