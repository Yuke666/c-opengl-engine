#ifndef ARTICULATED_FIGURE_DEF
#define ARTICULATED_FIGURE_DEF

#include "model.h"

typedef struct ConnectionAF ConnectionAF;

typedef struct {
	Vec3 	pos;
	Vec3 	vel;
	Vec3 	acc;
	float 	invMass;
} ParticleAF;

typedef struct {
	float 						cosAngle[2];
	float 						cosHalfAngle[2];
	float 						sinHalfAngle[2];
	float 						basis[9];
} XZRotationConstraint;

typedef struct {
	float 						cosAngle;
	float 						cosHalfAngle;
	float 						sinHalfAngle;
	Vec3 						up;
} RotationConstraint;

struct ConnectionAF {

	ParticleAF 					*p1;
	ParticleAF 					*p2;

	Bone 						*bone;
	void 						(*AddForce)(ConnectionAF *this, Vec3 force);
	void 						(*Draw)(ConnectionAF *this);

	float 						minLen;
	float 						maxLen;
	float 						restLen;

	float 						radius;

	float 						basis[9];

	union {
		XZRotationConstraint 	xzConstraint;
		RotationConstraint 		constraint;
	};
};

typedef struct {
	ParticleAF 					particles[MAX_BONES*2];
	ConnectionAF 				connections[MAX_BONES];
	int 						nConnections;
	int 						nParticles;
} ArticulatedFigure;


void ArticulatedFigure_FromSkeleton(ArticulatedFigure *figure, Skeleton skeleton);
void ArticulatedFigure_Update(ArticulatedFigure *figure, float dt);
void ArticulatedFigure_UpdateMatrices(ArticulatedFigure *figure, PlayingAnimation *anims, int nAnims);


#endif