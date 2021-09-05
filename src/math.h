#ifndef MATH_DEF
#define MATH_DEF

#include <math.h>
#include "game.h"

#define PI 3.1415
#define SWAP(x, y, T) do { T SWAP = x; x = y; y = SWAP; } while(0)
#define MIN(x,y) ( (x) < (y) ? (x) : (y) )
#define MAX(x,y) ((x)>(y)?(x):(y))
#define CLAMP(v, min, max) ( v > min ? (v > max ? max : v) : min )

extern const float math_Identity[16];

typedef struct {
    float x;
    float y;
    float w;
    float h;
} Rect2D;

typedef struct {
    float x;
    float y;
    float z;
    float w;
    float h;
    float d;
} Cube;

typedef struct {
    float x;
    float y;
} Vec2;

typedef struct {
    float x;
    float y;
    float z;
} Vec3;

typedef struct {
    Vec3 p1;
    Vec3 p2;
    Vec3 p3;
} Triangle;

typedef struct {
    Vec3 p1;
    Vec3 p2;
    float r;
} Capsule;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} Vec4;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} Quat;

typedef struct {
    Quat r;
    Quat d;
} DualQuat; 

typedef struct {
    Rect2D rect;
    Vec3 pos;
} BoundingBox;

typedef struct {
    Cube cube;
    Vec3 pos;
} BoundingCube;

typedef struct {
    float mag;
    Quat quat;
    Vec3 prevEuler;
    int bias;
    Vec3 minLimits;
    Vec3 maxLimits;
    Quat reference;
} Joint;

typedef struct {
    Joint *joints;
    int nJoints;
    Vec3 basePos;
} Arm;

extern const Vec3 math_Up;
extern const Vec3 math_Left;
extern const Vec3 math_Forward;

Vec3 Math_Vec3Normalize(Vec3 v);
Quat Math_QuatMult(Quat q1, Quat q2);
Quat Math_QuatNormalize(Quat q);
Quat Math_QuatInv(Quat q);
Quat Math_QuatConj(Quat q);
float Math_QuatMag(Quat q);
Vec3 Math_LerpVec3(Vec3 a1, Vec3 a2, float t);
float Math_Lerp(float a1, float a2, float t);
Vec3 Math_Vec3AddVec3(Vec3 v1, Vec3 v2);
Vec3 Math_Vec3MultVec3(Vec3 v1, Vec3 v2);
Vec3 Math_Vec3SubVec3(Vec3 v1, Vec3 v2);
Vec3 Math_Vec3MultFloat(Vec3 v, float s);
float Math_Vec3Magnitude(Vec3 v) ;
float Math_Vec3Dot(Vec3 v1, Vec3 v2);
Vec3 Math_Vec3Cross(Vec3 v1, Vec3 v) ;

int Math_SameSide(Vec3 p0, Vec3 p1, Vec3 a, Vec3 b);
Rect2D Math_BoundingBoxToRect2D(BoundingBox bb);
u8 Math_CheckCollisionRect2D(Rect2D r1, Rect2D r);
void Math_4x4to3x3(float *in, float *from);
void Math_QuatToMat3(Quat q, float *matrix);
void Math_TransposeMatrix3x3(float *matrix);
Vec3 Math_QuatRotate(Quat q, Vec3 v);
Quat Math_Slerp(Quat qa, Quat qb, float t);
void Math_InverseMatrix(float *m);
void Math_InverseMatrix3x3(float *m);
void Math_MatrixMatrixMult(float *res, float *a, float *b);
void Math_TranslateMatrix(float *matrix, Vec3 vector);
void Math_MatrixFromQuat(Quat q, float *matrix);
void Math_ScalingMatrixXYZ(float *matrix, float x, float y, float z);
void Math_Perspective(float *matrix, float fov, float a, float n, float f);
void Math_LookAt(float *ret, Vec3 eye, Vec3 center, Vec3 up );
void Math_RotateMatrix(float *matrix, Vec3 angles);
void Math_Ortho(float *matrix, float l, float r, float t, float b, float n, float f);
Vec2 Math_GetDistanceRect2D(Rect2D r1, Rect2D r2);
float Math_GetDistanceFloat(float min1, float max1, float min2, float max2);
Vec4 Math_MatrixMult4( Vec4 vert, float *matrix);
void Math_TransposeMatrix(float *matrix);
Quat Math_Quat(Vec3 v, float a);
Quat Math_QuatLookAt(Vec3 forward, Vec3 up);
Vec3 Math_MatrixMult3( Vec3 vert, float *matrix);
void Math_PlanarReflect(float *proj, float *view, Vec3 pos, Vec3 n, float fov, float a, float f);
float Math_Sign(float val);
void Math_AxisAngleToMatrix(float *matrix, Vec3 axis, float angle);
DualQuat Math_DualQuatTransform(Vec3 point, DualQuat dq);
DualQuat Math_DualQuatMult(DualQuat dq1, DualQuat dq2);
DualQuat Math_DualQuatConj(DualQuat dq);
Quat Math_QuatAdd(Quat q1, Quat q2);
Vec3 Math_DualQuatTranslation(DualQuat dq);
DualQuat Math_DualQuatRotTrans(Quat r, Vec3 v);
DualQuat Math_DualQuatTransRot(Quat r, Vec3 v);
void Math_InvTranspose(float *out, float *in);
float Math_GetDistancePointLineSegmentSqr(Vec3 point, Vec3 s1, Vec3 s2);
void Math_MatrixMatrixMult3x3(float *res, float *a, float *b);
void Math_OrthoNormalize3x3(float *matrix);
Vec3 Math_Vec3DivideFloat(Vec3 v, float s);
float DistanceSegmentSegment(Vec3 a1, Vec3 a2, Vec3 b1, Vec3 b2, float *s, float *t, Vec3 *p1, Vec3 *p2);

#endif