#include <string.h>
#include "math.h"

const float math_Identity[16] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};

const Vec3 math_Up = (Vec3){0,1,0};
const Vec3 math_Left = (Vec3){-1,0,0};
const Vec3 math_Forward = (Vec3){0,0,-1};

Vec4 Math_Vec4MultFloat(Vec4 v, float s){ return (Vec4){v.x * s, v.y * s, v.z * s, v.w * s}; }
Vec3 Math_Vec3AddVec3(Vec3 v1, Vec3 v2){ return (Vec3){v1.x + v2.x, v1.y + v2.y, v1.z + v2.z}; }
Vec3 Math_Vec3MultVec3(Vec3 v1, Vec3 v2){ return (Vec3){v1.x * v2.x, v1.y * v2.y, v1.z * v2.z}; }
Vec3 Math_Vec3SubVec3(Vec3 v1, Vec3 v2){ return (Vec3){v1.x - v2.x, v1.y - v2.y, v1.z - v2.z}; }
Vec3 Math_Vec3MultFloat(Vec3 v, float s){ return (Vec3){v.x * s, v.y * s, v.z * s}; }
Vec3 Math_Vec3DivideFloat(Vec3 v, float s){ return (Vec3){v.x / s, v.y / s, v.z / s}; }
float Math_Vec3Magnitude(Vec3 v) { return sqrt(v.x*v.x + v.y*v.y + v.z* v.z); }
float Math_Vec3Dot(Vec3 v1, Vec3 v2){ return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z); }
Vec3 Math_Vec3Cross(Vec3 v1, Vec3 v) { return (Vec3){(v1.y * v.z) - (v1.z * v.y), (v1.z * v.x) - (v1.x * v.z), (v1.x * v.y) - (v1.y * v.x) }; }
float Math_Lerp(float a1, float a2, float t){ return (a1 * (1.0 - t)) + (a2 * t); }

float Math_Sign(float val){
    if(val > 0) return 1;
    if(val < 0) return -1;
    return 0;
}

Vec3 Math_LerpVec3(Vec3 a1, Vec3 a2, float t){
    if(t > 1) return a2;
    if(t < 0) return a1;
    return Math_Vec3AddVec3(Math_Vec3MultFloat(a1,(1.0-t)), Math_Vec3MultFloat(a2, t));
}

Vec3 Math_Vec3Normalize(Vec3 v){
    float mag = Math_Vec3Magnitude(v);

    if(mag){
        v.x /= mag;
        v.y /= mag;
        v.z /= mag;
    }

    return v;
}

Quat Math_QuatConj(Quat q){ return (Quat){ -q.x, -q.y, -q.z, q.w }; }
float Math_QuatMag(Quat q){ return sqrtf(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w); }

Quat Math_QuatInv(Quat q){
    Quat conjugate = Math_QuatConj(q);
    double mag = pow(Math_QuatMag(q), 2);
    conjugate.x /= mag;
    conjugate.y /= mag;
    conjugate.z /= mag;
    conjugate.w /= mag;
    return conjugate;
}

Quat Math_QuatNormalize(Quat q){
    float mag = Math_QuatMag(q);
    return (Quat){
        q.x / mag,
        q.y / mag,
        q.z / mag,
        q.w / mag,
    };
}

Quat Math_QuatMult(Quat q1, Quat q2){
    return (Quat){
        q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
        q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x,
        q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w,
        q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z,
    };
}

DualQuat Math_DualQuatTransRot(Quat r, Vec3 v){

    DualQuat ret;

    r = Math_QuatNormalize(r);

    ret.r = r;

    ret.d = (Quat){
        0.5 * (r.w * v.x + r.y * v.z - r.z * v.y),
        0.5 * (r.w * v.y - r.x * v.z + r.z * v.x),
        0.5 * (r.w * v.z + r.x * v.y - r.y * v.x),
        0.5 * (- r.x * v.x - r.y * v.y - r.z * v.z),
    };

    return ret;
}

DualQuat Math_DualQuatRotTrans(Quat r, Vec3 t){

    DualQuat ret;

    r = Math_QuatNormalize(r);

    ret.r = r;

    ret.d = (Quat){
        0.5 * (+t.x * r.w + t.y * r.z - t.z * r.y),
        0.5 * (-t.x * r.z + t.y * r.w + t.z * r.x),
        0.5 * (+t.x * r.y - t.y * r.x + t.z * r.w),
        0.5 * (-t.x * r.x - t.y * r.y - t.z * r.z),
    };

    return ret;
}

Quat Math_QuatAdd(Quat q1, Quat q2){

    return (Quat){q1.x + q2.x, q1.y + q2.y, q1.z + q2.z, q1.w + q2.w};
}

DualQuat Math_DualQuatConj(DualQuat dq){

    return (DualQuat){Math_QuatConj(dq.r), Math_QuatConj(dq.d)};
}

DualQuat Math_DualQuatInv(DualQuat dq){

    return (DualQuat){Math_QuatInv(dq.r), Math_QuatInv(dq.d)};
}

DualQuat Math_DualQuatNormalize(DualQuat dq){

    float mag = Math_QuatMag(dq.r);

    dq.r.x /= mag;
    dq.r.y /= mag;
    dq.r.z /= mag;
    dq.r.w /= mag;

    dq.d.x /= mag;
    dq.d.y /= mag;
    dq.d.z /= mag;
    dq.d.w /= mag;

    return dq;
}

DualQuat Math_DualQuatMult(DualQuat dq1, DualQuat dq2){

    dq1 = Math_DualQuatNormalize(dq1);
    dq2 = Math_DualQuatNormalize(dq2);

    DualQuat ret;

    ret.r = Math_QuatMult(dq2.r, dq1.r);
    ret.d = Math_QuatAdd(Math_QuatMult(dq2.d, dq1.r), Math_QuatMult(dq2.r, dq1.d));

    return ret;
}

Vec3 Math_DualQuatTranslation(DualQuat dq){

    Quat d = (Quat){dq.d.x * 2, dq.d.y * 2, dq.d.z * 2, dq.d.w * 2};

    Quat t = Math_QuatMult(d, Math_QuatConj(dq.r));

    return (Vec3){t.x, t.y, t.z};
}

DualQuat Math_DualQuatTransform(Vec3 point, DualQuat dq){

    DualQuat pointDQ = (DualQuat){(Quat){0,0,0,1}, (Quat){point.x,point.y,point.z,0}};

    return Math_DualQuatMult(Math_DualQuatMult(dq, pointDQ), Math_DualQuatInv(dq));
}

u8 Math_CheckCollisionRect2D(Rect2D r1, Rect2D r){

    if(r1.x <= r.x + r.w  && r1.w  + r1.x >= r.x &&
       r1.y <= r.y + r.h && r1.h + r1.y >= r.y) return 1;

    return 0;
}

Rect2D Math_BoundingBoxToRect2D(BoundingBox bb){

    return (Rect2D){bb.rect.x + bb.pos.x, bb.rect.y + bb.pos.y, bb.rect.w, bb.rect.h};
}

int Math_SameSide(Vec3 p0, Vec3 p1, Vec3 a, Vec3 b){
    Vec3 cp0 = Math_Vec3Cross(Math_Vec3SubVec3(b,a), Math_Vec3SubVec3(p0,a));
    Vec3 cp1 = Math_Vec3Cross(Math_Vec3SubVec3(b,a), Math_Vec3SubVec3(p1,a));
    if(Math_Vec3Dot(cp0, cp1) >= 0) return 1;
    return 0;
}

float Math_GetDistanceFloat(float min1, float max1, float min2, float max2){

    if(max1 < min1) SWAP(min1, max1, float);
    if(max2 < min2) SWAP(min2, max2, float);

    float option1 = max1 - min2;
    float option2 = max2 - min1;
    float option3 = min1 - max2;

    if(min1 < min2)
        if(max1 <= max2)
            return option1;
        else
            if(option1 < option2) return option1; else return -option2;
    else
        if(max1 >= max2)
            return option3;
        else
            if(option1 < option2) return option1; else return -option2;

    return 0;
}

Vec2 Math_GetDistanceRect2D(Rect2D r1, Rect2D r2){
    
    return (Vec2){ Math_GetDistanceFloat(r1.x, r1.x+r1.w, r2.x, r2.x+r2.w), Math_GetDistanceFloat(r1.y, r1.y+r1.h, r2.y, r2.y+r2.h) };
}

float DistanceSegmentSegment(Vec3 a1, Vec3 a2, Vec3 b1, Vec3 b2, float *s, float *t, Vec3 *p1, Vec3 *p2){

    Vec3 v1 = Math_Vec3SubVec3(a2, a1);
    Vec3 v2 = Math_Vec3SubVec3(b2, b1);

    Vec3 r = Math_Vec3SubVec3(a1, b1);

    float sqMag1 = Math_Vec3Dot(v1, v1);
    float sqMag2 = Math_Vec3Dot(v2, v2);

    float f = Math_Vec3Dot(v2, r);
    float c = Math_Vec3Dot(v1, r);

    float b = Math_Vec3Dot(v1, v2);

    float denom = sqMag1*sqMag2 - b*b;
        
    if(denom != 0)
        *s = CLAMP((b*f - c*sqMag2) / denom, 0, 1);
    else
        *s = 0;

    float tnom = *s * b + f;

    if(tnom < 0){

        *t = 0;
        *s = CLAMP(-c / sqMag1, 0.0, 1.0);

    } else if(tnom > sqMag2){

        *t = 1.0;
        *s = CLAMP((b-c) / sqMag1, 0.0, 1.0);
    
    } else {

        *t = tnom / sqMag2;
    }

    *p1 = Math_Vec3AddVec3(a1, Math_Vec3MultFloat(v1, *s));
    *p2 = Math_Vec3AddVec3(a2, Math_Vec3MultFloat(v2, *t));

    return Math_Vec3Dot(Math_Vec3SubVec3(*p1, *p2), Math_Vec3SubVec3(*p2, *p1));
}

float Math_GetDistancePointLineSegmentSqr(Vec3 point, Vec3 s1, Vec3 s2){

    Vec3 pToP2 = Math_Vec3SubVec3(point, s2);
    Vec3 pToP1 = Math_Vec3SubVec3(point, s1);

    float dot1 = Math_Vec3Dot(pToP2, pToP1); 

    if(dot1 < 0) return Math_Vec3Dot(pToP2, pToP2);

    float dot2 = Math_Vec3Dot(pToP1, pToP1);


    if(dot2 <= dot1){
        Vec3 s1ToS2 = Math_Vec3SubVec3(s1, s2);
        return Math_Vec3Dot(s1ToS2, s1ToS2);
    }

    return Math_Vec3Dot(pToP2, pToP2) - dot1 * dot1 / dot2;
}

void Math_PlanarReflect(float *proj, float *view, Vec3 pos, Vec3 n, float fov, float a, float f){

    float projInvTrans[16], invView[16];

    float d = -Math_Vec3Dot(n, pos);

    float matrix[] = {
        1 - (2*n.x*n.x), -2*n.x*n.y, -2*n.x*n.z, -2*d*n.x,
        -2*n.x*n.y, 1 - (2*n.y*n.y), -2*n.y*n.z, -2*d*n.y,
        -2*n.x*n.z, -2*n.z*n.y, 1 - (2*n.z*n.z), -2*d*n.z,
        0,0,0,1
    };

    Math_Perspective(proj, fov, a, 1.0f, f);

    Math_InverseMatrix(view);

    Math_MatrixMatrixMult(view, matrix, view);

    Math_InverseMatrix(view);

    memcpy(invView, view, sizeof(float) * 16);
    Math_InverseMatrix(invView);
    Math_TransposeMatrix(invView);

    memcpy(projInvTrans, proj, sizeof(float) * 16);
    Math_InverseMatrix(projInvTrans);
    Math_TransposeMatrix(projInvTrans);

    Vec4 planeVec = (Vec4){n.x, n.y, n.z, d};
    planeVec = Math_MatrixMult4(planeVec, invView);

    Vec4 q = Math_MatrixMult4((Vec4){Math_Sign(planeVec.x), Math_Sign(planeVec.y), 1, 1}, projInvTrans);

    Vec4 c = Math_Vec4MultFloat(planeVec, 2.0f / ((planeVec.x * q.x) + (planeVec.y * q.y) + (planeVec.z * q.z) + (planeVec.w * q.w)));

    proj[8] = c.x;
    proj[9] = c.y;
    proj[10] = c.z + 1;
    proj[11] = c.w;
}

void Math_AxisAngleToMatrix(float *matrix, Vec3 axis, float angle){

    double s = sin(angle);
    double c = cos(angle);
    double t = 1 - c;

    axis = Math_Vec3Normalize(axis);

    Vec3 angles;

    if ((axis.x*axis.y*t + axis.z*s) > 0.998) { // north pole singularity detected
    
        angles.y = 2*atan2(axis.x*sin(angle/2),cos(angle/2));
        angles.z = PI/2;
        angles.x = 0;
    
    } else if ((axis.x*axis.y*t + axis.z*s) < -0.998) { // south pole singularity detected

        angles.y = -2*atan2(axis.x*sin(angle/2),cos(angle/2));
        angles.z = -PI/2;
        angles.x = 0;

    } else {

        angles.y = atan2(axis.y * s- axis.x * axis.z * t , 1 - (axis.y*axis.y+ axis.z*axis.z ) * t);
        angles.z = asin(axis.x * axis.y * t + axis.z * s) ;
        angles.x = atan2(axis.x * s - axis.y * axis.z * t , 1 - (axis.x*axis.x + axis.z*axis.z) * t);
    }

    Math_RotateMatrix(matrix, angles);
}

Quat Math_QuatLookAt(Vec3 forward, Vec3 up){

    Vec3 forwardW = {0,0,-1};

    up = Math_Vec3Normalize(up);

    Vec3 axis = Math_Vec3Normalize(Math_Vec3Cross(forward, forwardW));

    double theta = acos(Math_Vec3Dot(forward, forwardW));

    Vec3 b = Math_Vec3Normalize(Math_Vec3Cross(axis, forwardW));

    if(Math_Vec3Dot(b, forward) < 0) theta = -theta;

    Quat qb = Math_Quat(axis, theta);

    Vec3 upL = Math_Vec3Normalize(Math_QuatRotate(qb, up));
    Vec3 right = Math_Vec3Normalize(Math_Vec3Cross(forward, up));
    Vec3 upW = Math_Vec3Normalize(Math_Vec3Cross(right, forward));

    axis = Math_Vec3Cross(upL, upW);

    theta = acos(Math_Vec3Dot(upL, upW));

    // b = Math_Vec3Cross(axis, upW);

    // if(Math_Vec3Dot(b, upL) < 0) theta = -theta;

    return Math_QuatMult(Math_Quat(axis, theta), qb);
}

Quat Math_Quat(Vec3 v, float a){
    Quat q;
    q.x = v.x * sin(a/2);
    q.y = v.y * sin(a/2);
    q.z = v.z * sin(a/2);
    q.w = cos(a/2);
    return q;
}

Vec3 Math_QuatRotate(Quat q, Vec3 v){

    Quat q1 = Math_QuatMult(q, (Quat){v.x, v.y, v.z, 0});

    q1 = Math_QuatMult(q1, (Quat){-q.x, -q.y, -q.z, q.w});

    return (Vec3){q1.x, q1.y, q1.z};
}

Quat Math_Slerp(Quat qa, Quat qb, float t){

    Quat qm;
    
    float cosHalfTheta = qa.w*qb.w + qa.x*qb.x + qa.y*qb.y + qa.z*qb.z;
    
    if(cosHalfTheta < 0){
        qb.x *= -1; qb.y *= -1; qb.z *= -1; qb.w *= -1;
        cosHalfTheta *= -1; 
    }
    
    if(fabs(cosHalfTheta) >= 1.0){
        qm.w = qa.w; 
        qm.x = qa.x; 
        qm.y = qa.y;
        qm.z = qa.z;
        return qm;
    }
    
    float halfTheta = acos(cosHalfTheta);
    float sinHalfTheta = sin(halfTheta);
    
    if(fabs(sinHalfTheta) < 0.001){
        qm.w = (qa.w * 0.5 + qb.w * 0.5);
        qm.x = (qa.x * 0.5 + qb.x * 0.5);
        qm.y = (qa.y * 0.5 + qb.y * 0.5);
        qm.z = (qa.z * 0.5 + qb.z * 0.5);
        return qm;
    }
    
    float rA = sin((1-t) * halfTheta) / sinHalfTheta;
    float rB = sin(t * halfTheta) / sinHalfTheta;

    qm.x = (qa.x * rA + qb.x * rB);
    qm.y = (qa.y * rA + qb.y * rB);
    qm.z = (qa.z * rA + qb.z * rB);
    qm.w = (qa.w * rA + qb.w * rB);
 
    return qm;
}

void Math_MatrixFromQuat(Quat q, float *matrix){

    matrix[0] = 1.0f - 2.0f*q.y*q.y - 2.0f*q.z*q.z;
    matrix[1] = 2.0f*q.x*q.y - 2.0f*q.z*q.w;
    matrix[2] = 2.0f*q.x*q.z + 2.0f*q.y*q.w;
    matrix[3] = 0.0f;
    matrix[4] = 2.0f*q.x*q.y + 2.0f*q.z*q.w;
    matrix[5] = 1.0f - 2.0f*q.x*q.x - 2.0f*q.z*q.z;
    matrix[6] = 2.0f*q.y*q.z - 2.0f*q.x*q.w;
    matrix[7] = 0.0f;
    matrix[8] = 2.0f*q.x*q.z - 2.0f*q.y*q.w;
    matrix[9] = 2.0f*q.y*q.z + 2.0f*q.x*q.w;
    matrix[10] = 1.0f - 2.0f*q.x*q.x - 2.0f*q.y*q.y;
    matrix[11] = 0.0f;
    matrix[12] = 0.0f;
    matrix[13] = 0.0f;
    matrix[14] = 0.0f;
    matrix[15] = 1.0f;
}

void Math_QuatToMat3(Quat q, float *matrix){

    float yy = q.y * q.y;
    float xy = q.x * q.y;
    float zz = q.z * q.z;
    float zw = q.z * q.w;
    float xz = q.x * q.z;
    float yw = q.y * q.w;
    float xw = q.x * q.w;
    float yz = q.y * q.z;
    float xx = q.x * q.x;

    matrix[0] = 1.0f    - 2.0f*yy - 2.0f*zz;
    matrix[1] = 2.0f*xy - 2.0f*zw;
    matrix[2] = 2.0f*xz + 2.0f*yw;
    matrix[3] = 2.0f*xy + 2.0f*zw;
    matrix[4] = 1.0f    - 2.0f*xx - 2.0f*zz;
    matrix[5] = 2.0f*yz - 2.0f*xw;
    matrix[6] = 2.0f*xz - 2.0f*yw;
    matrix[7] = 2.0f*yz + 2.0f*xw;
    matrix[8] = 1.0f    - 2.0f*xx - 2.0f*yy;
}

void Math_TransposeMatrix(float *matrix){
    SWAP(matrix[1], matrix[4], float);
    SWAP(matrix[2], matrix[8], float);
    SWAP(matrix[3], matrix[12], float);
    SWAP(matrix[6], matrix[9], float);
    SWAP(matrix[7], matrix[13], float);
    SWAP(matrix[11], matrix[14], float);
}

void Math_TransposeMatrix3x3(float *matrix){
    SWAP(matrix[1], matrix[3], float);
    SWAP(matrix[2], matrix[6], float);
    SWAP(matrix[5], matrix[7], float);
}

void Math_TranslateMatrix(float *matrix, Vec3 v){
    matrix[0] = 1;
    matrix[1] = 0;
    matrix[2] = 0;
    matrix[3] = v.x;
    matrix[4] = 0;
    matrix[5] = 1;
    matrix[6] = 0;
    matrix[7] = v.y;
    matrix[8] = 0;
    matrix[9] = 0;
    matrix[10] = 1;
    matrix[11] = v.z;
    matrix[12] = 0;
    matrix[13] = 0;
    matrix[14] = 0;
    matrix[15] = 1;
}

Vec4 Math_MatrixMult4( Vec4 vert, float *matrix){
    Vec4 out;
    out.x = ((vert.x * matrix[0])  + (vert.y * matrix[1])  + (vert.z * matrix[2])  + (vert.w * matrix[3]));
    out.y = ((vert.x * matrix[4])  + (vert.y * matrix[5])  + (vert.z * matrix[6])  + (vert.w * matrix[7]));
    out.z = ((vert.x * matrix[8])  + (vert.y * matrix[9])  + (vert.z * matrix[10]) + (vert.w * matrix[11]));
    out.w = ((vert.x * matrix[12]) + (vert.y * matrix[13]) + (vert.z * matrix[14]) + (vert.w * matrix[15]));
    return out;
}

Vec3 Math_MatrixMult3( Vec3 vert, float *matrix){
    Vec3 out;
    out.x = ((vert.x * matrix[0])  + (vert.y * matrix[1])  + (vert.z * matrix[2]));
    out.y = ((vert.x * matrix[4])  + (vert.y * matrix[5])  + (vert.z * matrix[6]));
    out.z = ((vert.x * matrix[8])  + (vert.y * matrix[9])  + (vert.z * matrix[10]));
    return out;
}

void Math_MatrixMatrixMult(float *res, float *a, float *b){
    
    float m[16];

    m[0] = (a[0]  * b[0]) + (a[1]  *  b[4]) + (a[2]  * b[8])  + (a[3]  * b[12]);
    m[1] = (a[0]  * b[1]) + (a[1]  *  b[5]) + (a[2]  * b[9])  + (a[3]  * b[13]);
    m[2] = (a[0]  * b[2]) + (a[1]  *  b[6]) + (a[2]  * b[10]) + (a[3]  * b[14]);
    m[3] = (a[0]  * b[3]) + (a[1]  *  b[7]) + (a[2]  * b[11]) + (a[3]  * b[15]);
    m[4] = (a[4]  * b[0]) + (a[5]  *  b[4]) + (a[6]  * b[8])  + (a[7]  * b[12]);
    m[5] = (a[4]  * b[1]) + (a[5]  *  b[5]) + (a[6]  * b[9])  + (a[7]  * b[13]);
    m[6] = (a[4]  * b[2]) + (a[5]  *  b[6]) + (a[6]  * b[10]) + (a[7]  * b[14]);
    m[7] = (a[4]  * b[3]) + (a[5]  *  b[7]) + (a[6]  * b[11]) + (a[7]  * b[15]);
    m[8] = (a[8]  * b[0]) + (a[9]  *  b[4]) + (a[10] * b[8])  + (a[11] * b[12]);
    m[9] = (a[8]  * b[1]) + (a[9]  *  b[5]) + (a[10] * b[9])  + (a[11] * b[13]);
    m[10] = (a[8]  * b[2]) + (a[9]  *  b[6]) + (a[10] * b[10]) + (a[11] * b[14]);
    m[11] = (a[8]  * b[3]) + (a[9]  *  b[7]) + (a[10] * b[11]) + (a[11] * b[15]);
    m[12] = (a[12] * b[0]) + (a[13] *  b[4]) + (a[14] * b[8])  + (a[15] * b[12]);
    m[13] = (a[12] * b[1]) + (a[13] *  b[5]) + (a[14] * b[9])  + (a[15] * b[13]);
    m[14] = (a[12] * b[2]) + (a[13] *  b[6]) + (a[14] * b[10]) + (a[15] * b[14]);
    m[15] = (a[12] * b[3]) + (a[13] *  b[7]) + (a[14] * b[11]) + (a[15] * b[15]);

    memcpy(res, m, sizeof(float) * 16);
}

void Math_MatrixMatrixMult3x3(float *res, float *a, float *b){
    
    float m[16];

    m[0] = (a[0] * b[0]) + (a[1] * b[3]) + (a[2] * b[6]);
    m[1] = (a[0] * b[1]) + (a[1] * b[4]) + (a[2] * b[7]);
    m[2] = (a[0] * b[2]) + (a[1] * b[5]) + (a[2] * b[8]);
    m[3] = (a[3] * b[0]) + (a[4] * b[3]) + (a[5] * b[6]);
    m[4] = (a[3] * b[1]) + (a[4] * b[4]) + (a[5] * b[7]);
    m[5] = (a[3] * b[2]) + (a[4] * b[5]) + (a[5] * b[8]);
    m[6] = (a[6] * b[0]) + (a[7] * b[3]) + (a[8] * b[6]);
    m[7] = (a[6] * b[1]) + (a[7] * b[4]) + (a[8] * b[7]);
    m[8] = (a[6] * b[2]) + (a[7] * b[5]) + (a[8] * b[8]);

    memcpy(res, m, sizeof(float) * 16);
}

void Math_InverseMatrix(float *m){

    float m00 = m[ 0], m01 = m[ 1], m02 = m[ 2], m03 = m[ 3];
    float m10 = m[ 4], m11 = m[ 5], m12 = m[ 6], m13 = m[ 7];
    float m20 = m[ 8], m21 = m[ 9], m22 = m[10], m23 = m[11];
    float m30 = m[12], m31 = m[13], m32 = m[14], m33 = m[15];

    m[ 0] = m12*m23*m31 - m13*m22*m31 + m13*m21*m32 - m11*m23*m32 - m12*m21*m33 + m11*m22*m33;
    m[ 1] = m03*m22*m31 - m02*m23*m31 - m03*m21*m32 + m01*m23*m32 + m02*m21*m33 - m01*m22*m33;
    m[ 2] = m02*m13*m31 - m03*m12*m31 + m03*m11*m32 - m01*m13*m32 - m02*m11*m33 + m01*m12*m33;
    m[ 3] = m03*m12*m21 - m02*m13*m21 - m03*m11*m22 + m01*m13*m22 + m02*m11*m23 - m01*m12*m23;
    m[ 4] = m13*m22*m30 - m12*m23*m30 - m13*m20*m32 + m10*m23*m32 + m12*m20*m33 - m10*m22*m33;
    m[ 5] = m02*m23*m30 - m03*m22*m30 + m03*m20*m32 - m00*m23*m32 - m02*m20*m33 + m00*m22*m33;
    m[ 6] = m03*m12*m30 - m02*m13*m30 - m03*m10*m32 + m00*m13*m32 + m02*m10*m33 - m00*m12*m33;
    m[ 7] = m02*m13*m20 - m03*m12*m20 + m03*m10*m22 - m00*m13*m22 - m02*m10*m23 + m00*m12*m23;
    m[ 8] = m11*m23*m30 - m13*m21*m30 + m13*m20*m31 - m10*m23*m31 - m11*m20*m33 + m10*m21*m33;
    m[ 9] = m03*m21*m30 - m01*m23*m30 - m03*m20*m31 + m00*m23*m31 + m01*m20*m33 - m00*m21*m33;
    m[10] = m01*m13*m30 - m03*m11*m30 + m03*m10*m31 - m00*m13*m31 - m01*m10*m33 + m00*m11*m33;
    m[11] = m03*m11*m20 - m01*m13*m20 - m03*m10*m21 + m00*m13*m21 + m01*m10*m23 - m00*m11*m23;
    m[12] = m12*m21*m30 - m11*m22*m30 - m12*m20*m31 + m10*m22*m31 + m11*m20*m32 - m10*m21*m32;
    m[13] = m01*m22*m30 - m02*m21*m30 + m02*m20*m31 - m00*m22*m31 - m01*m20*m32 + m00*m21*m32;
    m[14] = m02*m11*m30 - m01*m12*m30 - m02*m10*m31 + m00*m12*m31 + m01*m10*m32 - m00*m11*m32;
    m[15] = m01*m12*m20 - m02*m11*m20 + m02*m10*m21 - m00*m12*m21 - m01*m10*m22 + m00*m11*m22;

    float invDet = 1.0 / ((m00 * m[0]) + (m01 * m[4]) + (m02 * m[8]) + (m03 * m[12])); 

    int k;
    for(k = 0; k < 16; k++) m[k] *= invDet;
}

void Math_InverseMatrix3x3(float *m){

    float m00 = m[0], m01 = m[1], m02 = m[2];
    float m10 = m[3], m11 = m[4], m12 = m[5];
    float m20 = m[6], m21 = m[7], m22 = m[8];

    m[0] = - m12*m21 + m11*m22;
    m[1] = + m02*m21 - m01*m22;
    m[2] = - m02*m11 + m01*m12;
    m[3] = + m12*m20 - m10*m22;
    m[4] = - m02*m20 + m00*m22;
    m[5] = + m02*m10 - m00*m12;
    m[6] = - m11*m20 + m10*m21;
    m[7] = + m01*m20 - m00*m21;
    m[8] = - m01*m10 + m00*m11;

    float invDet = 1.0f / ((m00 * m[0]) + (m01 * m[3]) + (m02 * m[6]));

    int k;
    for(k = 0; k < 9; k++) m[k] *= invDet;
}

void Math_4x4to3x3(float *in, float *from){
    in[0] = from[0];
    in[1] = from[1];
    in[2] = from[2];
    in[3] = from[4];
    in[4] = from[5];
    in[5] = from[6];
    in[6] = from[8];
    in[7] = from[9];
    in[8] = from[10];
}

void Math_InvTranspose(float *in, float *from){
    Math_4x4to3x3(in, from);
    Math_InverseMatrix3x3(in);
    Math_TransposeMatrix3x3(in);
}

void Math_ScalingMatrixXYZ(float *matrix, float x, float y, float z){
    matrix[0] = x;
    matrix[1] = 0;
    matrix[2] = 0;
    matrix[3] = 0;
    matrix[4] = 0;
    matrix[5] = y;
    matrix[6] = 0;
    matrix[7] = 0;
    matrix[8] = 0;
    matrix[9] = 0;
    matrix[10] = z;
    matrix[11] = 0;
    matrix[12] = 0;
    matrix[13] = 0;
    matrix[14] = 0;
    matrix[15] = 1;
}

void Math_Perspective(float *matrix, float fov, float a, float n, float f){

    // float b = tan(fov/2) * n;
    // float t = -b;
    // float l = -b*a;
    // float r =  b*a;

    // matrix[0] = (2*n)/(r-l);
    // matrix[1] = 0;
    // matrix[2] = (r+l)/(r-l);
    // matrix[3] = 0;
    // matrix[4] = 0;
    // matrix[5] = (2*n)/(t-b);
    // matrix[6] = (t+b)/(t-b);
    // matrix[7] = 0;
    // matrix[8] = 0;
    // matrix[9] = 0;
    // matrix[10] = -(f+n)/(f-n);
    // matrix[11] = -(2*f*n)/(f-n);
    // matrix[12] = 0;
    // matrix[13] = 0;
    // matrix[14] = -1;
    // matrix[15] = 0;

    float tanHalfFov = tan(fov * 0.5f);

    matrix[0] = 1.0 / (a * tanHalfFov);
    matrix[1] = 0;
    matrix[2] = 0;
    matrix[3] = 0;
    matrix[4] = 0;
    matrix[5] = 1.0 / tanHalfFov;
    matrix[6] = 0;
    matrix[7] = 0;
    matrix[8] = 0;
    matrix[9] = 0;
    // matrix[10] = -((-n-f) / (n-f));
    // matrix[10] = (n+f)/(n-f);
    matrix[10] = -(((-n-f)/(n-f)) / (n-f));
    matrix[11] = (2*f*n) / (n-f);
    matrix[12] = 0;
    matrix[13] = 0;
    matrix[14] = -1.0;
    matrix[15] = 0.0f;

    // float two[16];

    // memcpy(two, matrix, sizeof(float) * 16);
    // Math_InverseMatrix(two);

    // printf("%f %f %f %f\n", two[0], two[1], two[2], two[3]);
    // printf("%f %f %f %f\n", two[4], two[5], two[6], two[7]);
    // printf("%f %f %f %f\n", two[8], two[9], two[10], two[11]);
    // printf("%f %f %f %f\n", two[12], two[13], two[14], two[15]);

    // printf("=======\n");

    // printf("%f %f %f %f\n", matrix[0], matrix[1], matrix[2], matrix[3]);
    // printf("%f %f %f %f\n", matrix[4], matrix[5], matrix[6], matrix[7]);
    // printf("%f %f %f %f\n", matrix[8], matrix[9], matrix[10], matrix[11]);
    // printf("%f %f %f %f\n", matrix[12], matrix[13], matrix[14], matrix[15]);

    // // // m[0] = -m11*m23*m32;
    // // // m[5] = -m00*m23*m32;

    // printf("=======\n");


    // printf("%f\n", 1.0f / matrix[5] );
    // printf("%f\n", 1.0f / matrix[0] );

    // // -0.0501 = matrix[10] / pmatrix[11];
    // // -4.995 = 1.0 / matrix[11];

    // printf("=======\n");

    // printf("%f\n", matrix[10] / matrix[11]);
    // printf("%f\n", 1.0 / (matrix[11]));

    // Vec4 vec = Math_MatrixMult4((Vec4){0,0,50,1}, matrix);

    // float z = -1.0f / (((vec.z/vec.w) * (1.0f/matrix[11])) + (matrix[10]/matrix[11]));

    // printf("%f %f\n", z, vec.z/vec.w);

    // vec.x /= vec.w;
    // vec.y /= vec.w;
    // vec.z /= vec.w;
    // vec.w /= vec.w;

    // vec = Math_MatrixMult4(vec, two);

    // vec.x /= vec.w;
    // vec.y /= vec.w;
    // vec.z /= vec.w;

    // printf("%f %f %f\n",vec.x, vec.y, vec.z );


}

void Math_LookAt(float *ret, Vec3 eye, Vec3 center, Vec3 up){

    Vec3 z = Math_Vec3Normalize(Math_Vec3SubVec3(eye, center));  // Forward
    // Vec3 x = Math_Vec3Normalize(Math_Vec3Cross(up, z)); // Right
    Vec3 x = Math_Vec3Normalize(Math_Vec3Cross(up, z)); // Right
    Vec3 y = Math_Vec3Normalize(Math_Vec3Cross(z, x));

    ret[0] = x.x;
    ret[1] = x.y;
    ret[2] = x.z;
    ret[3] = -(Math_Vec3Dot(x, eye));
    ret[4] = y.x;
    ret[5] = y.y;
    ret[6] = y.z;
    ret[7] = -(Math_Vec3Dot(y, eye));
    ret[8] =  z.x;
    ret[9] =  z.y;
    ret[10] = z.z;
    ret[11] = -(Math_Vec3Dot(z, eye));
    ret[12] = 0;
    ret[13] = 0;
    ret[14] = 0;
    ret[15] = 1;
}

void Math_RotateMatrix(float *matrix, Vec3 angles){

    float sx = sin(angles.x);
    float cx = cos(angles.x);
    float sy = sin(angles.y);
    float cy = cos(angles.y);
    float sz = sin(angles.z);
    float cz = cos(angles.z);

    matrix[0] = cy*cz;
    matrix[1] = (-cy*sz*cx) + (sy*sx);
    matrix[2] = (cy*sz*sx) + (sy*cx);
    matrix[3] = 0;
    matrix[4] = sz;
    matrix[5] = cz*cx;
    matrix[6] = -cz*sx;
    matrix[7] = 0;
    matrix[8] = -sy*cz;
    matrix[9] = (sy*sz*cx) + (cy*sx);
    matrix[10] = (-sy*sz*sx) + (cy*cx);
    matrix[11] = 0;
    matrix[12] = 0;
    matrix[13] = 0;
    matrix[14] = 0;
    matrix[15] = 1;
}

void Math_OrthoNormalize3x3(float *matrix){

    Vec3 *left      = (Vec3 *)&matrix[0];
    Vec3 *up        = (Vec3 *)&matrix[3];
    Vec3 *forward   = (Vec3 *)&matrix[6];

    *left       = Math_Vec3Normalize(*left);
    *forward    = Math_Vec3Normalize(Math_Vec3Cross(*left, *up));
    *up         = Math_Vec3Normalize(Math_Vec3Cross(*forward, *left));
}

void Math_Ortho(float *matrix, float l, float r, float t, float b, float n, float f){
    
    // float m[16] = {
    //     2/(r-l),    0,           0,             -((r+l)/(r-l)),
    //     0,          2/(t-b),     0,             -((t+b)/(t-b)),
    //     0,          0,           1/(f-n),       n/(f-n),
    //     0,          0,           0,              1
    // };

    // memcpy(matrix, m, sizeof(m));

    matrix[0] = 2.0f/(r-l);    matrix[1] = 0.0f;           matrix[2] = 0.0f;            matrix[3] = -((r+l)/(r-l));
    matrix[4] = 0.0f;          matrix[5] = 2.0f/(t-b);     matrix[6] = 0.0f;            matrix[7] = -((t+b)/(t-b));
    matrix[8] = 0.0f;          matrix[9] = 0.0f;           matrix[10] = 1.0f/(f-n);     matrix[11] = n/(f-n);
    matrix[12] = 0.0f;         matrix[13] = 0.0f;          matrix[14] = 0.0f;           matrix[15] =  1.0f;
}