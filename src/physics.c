#include "physics.h"

void Physics_DebugRenderRigedBody(RigidBody *body){

}

void Physics_Integrate(RigidBody *body, float dt){

	if(!dt) return;

	body->pos = Math_Vec3AddVec3(body->pos, Math_Vec3MultFloat(body->vel, dt));

	float c[9];
	
	memcpy(c, body->orientation, sizeof(float) * 9);

	Vec3 w = body->angVel;

    body->orientation[0] += dt * (0 + (-w.z * c[3]) + (w.y * c[6]));
    body->orientation[1] += dt * (0 + (-w.z * c[4]) + (w.y * c[7]));
    body->orientation[2] += dt * (0 + (-w.z * c[5]) + (w.y * c[8]));
    body->orientation[3] += dt * ((w.z * c[0]) + 0 + (-w.x * c[6]));
    body->orientation[4] += dt * ((w.z * c[1]) + 0 + (-w.x * c[7]));
    body->orientation[5] += dt * ((w.z * c[2]) + 0 + (-w.x * c[8]));
    body->orientation[6] += dt * ((-w.y * c[0]) + (w.x * c[3]) + 0);
    body->orientation[7] += dt * ((-w.y * c[1]) + (w.x * c[4]) + 0);
    body->orientation[8] += dt * ((-w.y * c[2]) + (w.x * c[5]) + 0);

	Math_OrthoNormalize3x3(body->orientation);
	
	body->force = Math_Vec3DivideFloat(GRAVITY, body->invMass);

	body->vel = Math_Vec3AddVec3(body->vel, Math_Vec3MultFloat(body->force, dt * body->invMass));

	body->angMomentum = Math_Vec3AddVec3(body->angMomentum, Math_Vec3MultFloat(body->torque, dt));

	float *a = body->orientation;
	float *b = body->invInertiaTensor;

    c[0] = (a[0] * b[0]) + (a[1] * b[3]) + (a[2] * b[6]);
    c[1] = (a[0] * b[1]) + (a[1] * b[4]) + (a[2] * b[7]);
    c[2] = (a[0] * b[2]) + (a[1] * b[5]) + (a[2] * b[8]);
    c[3] = (a[3] * b[0]) + (a[4] * b[3]) + (a[5] * b[6]);
    c[4] = (a[3] * b[1]) + (a[4] * b[4]) + (a[5] * b[7]);
    c[5] = (a[3] * b[2]) + (a[4] * b[5]) + (a[5] * b[8]);
    c[6] = (a[6] * b[0]) + (a[7] * b[3]) + (a[8] * b[6]);
    c[7] = (a[6] * b[1]) + (a[7] * b[4]) + (a[8] * b[7]);
    c[8] = (a[6] * b[2]) + (a[7] * b[5]) + (a[8] * b[8]);

    body->invWorldInertiaTensor[0] = (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]);
    body->invWorldInertiaTensor[1] = (a[0] * b[3]) + (a[1] * b[4]) + (a[2] * b[5]);
    body->invWorldInertiaTensor[2] = (a[0] * b[6]) + (a[1] * b[7]) + (a[2] * b[8]);
    body->invWorldInertiaTensor[3] = (a[3] * b[0]) + (a[4] * b[1]) + (a[5] * b[2]);
    body->invWorldInertiaTensor[4] = (a[3] * b[3]) + (a[4] * b[4]) + (a[5] * b[5]);
    body->invWorldInertiaTensor[5] = (a[3] * b[6]) + (a[4] * b[7]) + (a[5] * b[8]);
    body->invWorldInertiaTensor[6] = (a[6] * b[0]) + (a[7] * b[1]) + (a[8] * b[2]);
    body->invWorldInertiaTensor[7] = (a[6] * b[3]) + (a[7] * b[4]) + (a[8] * b[5]);
    body->invWorldInertiaTensor[8] = (a[6] * b[6]) + (a[7] * b[7]) + (a[8] * b[8]);

    a = body->invWorldInertiaTensor;

    body->angVel.x = ((body->angMomentum.x * a[0]) + (body->angMomentum.y * a[1])  + (body->angMomentum.z * a[2]));
    body->angVel.y = ((body->angMomentum.x * a[3]) + (body->angMomentum.y * a[4])  + (body->angMomentum.z * a[5]));
    body->angVel.z = ((body->angMomentum.x * a[6]) + (body->angMomentum.y * a[7])  + (body->angMomentum.z * a[8]));
}

void Physics_AddForce(RigidBody *body, Vec3 point, Vec3 force){

	body->force = Math_Vec3AddVec3(body->force, force);
	body->torque = Math_Vec3AddVec3(body->torque, Math_Vec3Cross(Math_Vec3SubVec3(point, body->pos), force));
}

void Physics_InitRigidBody(RigidBody *body){

	memset(body->invInertiaTensor, 0, sizeof(float) * 16);

	int k;
	for(k = 0; k < body->nParticles; k++){

		Vec3 v = body->particles[k].pos;
		float m = body->particles[k].mass;

		body->invInertiaTensor[0] += m*((v.y*v.y)+(v.z*v.z));
		body->invInertiaTensor[1] += m*(-v.x*v.y);
		body->invInertiaTensor[2] += m*(-v.x*v.z);
		body->invInertiaTensor[3] += m*(-v.x*v.y);
		body->invInertiaTensor[4] += m*((v.x*v.x)+(v.z*v.z));
		body->invInertiaTensor[5] += m*(-v.y*v.z);
		body->invInertiaTensor[6] += m*(-v.x*v.z);
		body->invInertiaTensor[7] += m*(-v.y*v.z);
		body->invInertiaTensor[8] += m*((v.x*v.x)+(v.y*v.y));

		body->invMass += m;
	}

	body->invMass = 1.0 / body->invMass;

	// Math_InverseMatrix3x3(body->invInertiaTensor);

	body->orientation[0] = 1;
	body->orientation[4] = 1;
	body->orientation[8] = 1;
}