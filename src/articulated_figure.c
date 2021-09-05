#include "articulated_figure.h"
#include "math.h"

// typedef struct {
	// float 						cosAngle[2];
	// float 						cosHalfAngle[2];
	// float 						sinHalfAngle[2];
	// float 						basis[9];
// } XZRotationConstraint;

static void PyramidLimit(
	XZRotationConstraint *limit,
	float *pBasis,
	Vec3 vec){

	Vec3 c[3];

	float *d = &c[0].x;

	if(pBasis){
	
		float *a = pBasis;
		float *b = limit->basis;

	    d[0] = (a[0] * b[0]) + (a[1] * b[3]) + (a[2] * b[6]);
	    d[1] = (a[0] * b[1]) + (a[1] * b[4]) + (a[2] * b[7]);
	    d[2] = (a[0] * b[2]) + (a[1] * b[5]) + (a[2] * b[8]);
	    d[3] = (a[3] * b[0]) + (a[4] * b[3]) + (a[5] * b[6]);
	    d[4] = (a[3] * b[1]) + (a[4] * b[4]) + (a[5] * b[7]);
	    d[5] = (a[3] * b[2]) + (a[4] * b[5]) + (a[5] * b[8]);
	    d[6] = (a[6] * b[0]) + (a[7] * b[3]) + (a[8] * b[6]);
	    d[7] = (a[6] * b[1]) + (a[7] * b[4]) + (a[8] * b[7]);
	    d[8] = (a[6] * b[2]) + (a[7] * b[5]) + (a[8] * b[8]);

	} else {

		d = limit->basis;
	}

	Vec3 x = Math_Vec3SubVec3(vec, c[1]);

	float cosx = Math_Vec3Dot(vec, c[2]);	
	float cosy = Math_Vec3Dot(vec, c[2]);	

}


	// int i;
	// float a[2];
	// idVec6 J1row, J2row;
	// idMat3 worldBase;
	// idVec3 anchor, body1ax, ax[2], v, normal, pyramidVector, p1, p2;
	// idQuat q;
	// idAFBody *master;

	// if ( af_skipLimits.GetBool() ) {
	// 	lm.Zero();	// constraint exerts no force
	// 	return false;
	// }

	// physics = phys;
	// master = body2 ? body2 : physics->GetMasterBody();

	// if ( master ) {
	// 	worldBase[0] = pyramidBasis[0] * master->GetWorldAxis();
	// 	worldBase[1] = pyramidBasis[1] * master->GetWorldAxis();
	// 	worldBase[2] = pyramidBasis[2] * master->GetWorldAxis();
	// 	anchor = master->GetWorldOrigin() + pyramidAnchor * master->GetWorldAxis();
	// }
	// else {
	// 	worldBase = pyramidBasis;
	// 	anchor = pyramidAnchor;
	// }

	// body1ax = body1Axis * body1->GetWorldAxis();

	// for ( i = 0; i < 2; i++ ) {
	// 	ax[i] = body1ax - worldBase[!i] * body1ax * worldBase[!i];
	// 	ax[i].Normalize();
	// 	a[i] = worldBase[2] * ax[i];
	// }

	// // if the body1 axis is inside the pyramid
	// if ( a[0] > cosAngle[0] && a[1] > cosAngle[1] ) {
	// 	lm.Zero();	// constraint exerts no force
	// 	return false;
	// }

	// // calculate the inward pyramid normal for the position the body1 axis went outside the pyramid
	// pyramidVector = worldBase[2];
	// for ( i = 0; i < 2; i++ ) {
	// 	if ( a[i] <= cosAngle[i] ) {
	// 		v = ax[i].Cross( worldBase[2] );
	// 		v.Normalize();
	// 		q.x = v.x * sinHalfAngle[i];
	// 		q.y = v.y * sinHalfAngle[i];
	// 		q.z = v.z * sinHalfAngle[i];
	// 		q.w = cosHalfAngle[i];
	// 		pyramidVector *= q.ToMat3();
	// 	}
	// }
	// normal = pyramidVector.Cross( worldBase[2] ).Cross( pyramidVector );
	// normal.Normalize();

	// p1 = anchor + 32.0f * pyramidVector - body1->GetWorldOrigin();

	// J1row.SubVec3(0) = normal;
	// J1row.SubVec3(1) = p1.Cross( normal );
	// J1.Set( 1, 6, J1row.ToFloatPtr() );

	// c1[0] = (invTimeStep * LIMIT_ERROR_REDUCTION) * ( normal * (32.0f * body1ax) );

	// if ( body2 ) {

	// 	p2 = anchor + 32.0f * pyramidVector - master->GetWorldOrigin();

	// 	J2row.SubVec3(0) = -normal;
	// 	J2row.SubVec3(1) = p2.Cross( -normal );
	// 	J2.Set( 1, 6, J2row.ToFloatPtr() );

	// 	c2[0] = 0.0f;
	// }

	// lo[0] = 0.0f;
	// e[0] = LIMIT_LCP_EPSILON;

	// physics->AddFrameConstraint( this );

	// return true;


// void ArticulatedFigure_FromSkeleton(ArticulatedFigure *figure, Skeleton skeleton){

// 	int k;
// 	for(k = 0; k < skeleton.nBones; k++){

// 		figure->particles[k].pos.x = bone->absMatrix[3];
// 		figure->particles[k].pos.y = bone->absMatrix[7];
// 		figure->particles[k].pos.z = bone->absMatrix[11];
// 		figure->particles[k].vel = (Vec3){0,0,0};
// 		figure->particles[k].acc = (Vec3){0,0,0};
// 		figure->particles[k].invMass = 0;
// 	}
// }

// void ArticulatedFigure_Update(ArticulatedFigure *figure, float dt){

// }

// void ArticulatedFigure_UpdateMatrices(ArticulatedFigure *figure, PlayingAnimation *anims, int nAnims){

// }