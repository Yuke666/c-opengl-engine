#ifndef PHYSICS_DEF
#define PHYSICS_DEF

#include "math.h"

#define MAX_RIGID_BODY_PARTICLES 32
#define GRAVITY (Vec3){0,0.01,0}

typedef struct {
	float 		mass;
	Vec3 		pos;
} RigidBodyParticle;

typedef struct {
	float 					invMass;
	float 					invInertiaTensor[9];
	float 					invWorldInertiaTensor[9];
	float 					orientation[9];
	float 					coefOfRestitution;
	Vec3 					pos;
	Vec3 					vel;
	Vec3 					angMomentum;
	Vec3 					angVel;
	Vec3 					force;
	Vec3 					torque;
	void 					*data;
	RigidBodyParticle 		particles[MAX_RIGID_BODY_PARTICLES];
	int 					nParticles;
} RigidBody;


void Physics_DebugRenderRigedBody(RigidBody *body);
void Physics_Integrate(RigidBody *body, float dt);
void Physics_InitRigidBody(RigidBody *body);


#endif