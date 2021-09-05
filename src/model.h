#ifndef MODEL_DEF
#define MODEL_DEF

#include "game.h"
#include "math.h"
#include "memory.h"

#define MAX_MODEL_MATERIALS 	64
#define MAX_BONES 				64
#define BONE_MAX_CHILDREN 		6
#define ANIMATION_FRAME_RATE 	60

typedef struct Bone Bone;

struct Bone {
	u8 				index;
	u8 				nChildren;
	Bone 			*parent;
	Vec3 			pos;
	Quat 			rot;
	Bone 			*children[BONE_MAX_CHILDREN];
	float 			absMatrix[16];
	float 			invBindMatrix[16];
	Cube 			cube;
	float 			damping;
	float 			spring;
	Vec3 			angVel;
	Quat 			rotDisplacement;
	// Vec3 			modelPos;
	// Vec3 			modelBasis[3];
};


typedef struct {
	u8 				boneIndex;
	int 			frame;
	Vec3 			pos;
	Quat 			rot;
} Keyframe;

typedef struct {
	u8 				nKeyframes[MAX_BONES];
	int 			length;
	Keyframe 		*keyframes[MAX_BONES];
} Animation;

typedef struct {
	u8 				active;
	float 			weight;
	float 			weightSpeed;
	float 			into;
	Animation 		*anim;
} PlayingAnimation;

typedef struct {
	u8 				nBones;
	Vec4 			matrices[MAX_BONES * 3];
	Bone 			bones[MAX_BONES];
	Bone 			*root;
} Skeleton;

typedef struct {
	u32				useSSS;
	u32 			texture;
	u32 			normalTexture;
	float 			specularHardness;
	float 			ambient;
	Vec4 			diffuse;
	Vec4 			specular;
} Material;

typedef struct {
	u8 				nMaterials;
	u8 				nTextures;
	u8 				nNormalTextures;
	u16 			nElements[MAX_MODEL_MATERIALS];
	Material 		materials[MAX_MODEL_MATERIALS];
	u32 			vao;
	u32 			vbo;
	u32 			ebo;
} Model;

void Animation_Load(Animation *animation, const char *path);
void Animation_Free(Animation animation);
void RiggedModel_Free(Model *model);
void RiggedModel_Load(Model *model, Skeleton *skeleton, const char *path);
void Skeleton_Update(Skeleton *skeleton, PlayingAnimation *anims, int nAnims);
void Skeleton_UpdateSprings(Skeleton *skeleton);
void Model_Free(Model *model);
void Model_Load(Model *model, const char *path);
void Model_DeleteTextures(Model *model);
void Skeleton_BlendAnims(PlayingAnimation *anims, int nAnims, float dt);


#endif