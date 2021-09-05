#include <GL/glew.h>
#include <string.h>
#include "deflate.h"
#include "log.h"
#include "model.h"
#include "shader_files.h"
#include "shaders.h"
#include "math.h"

static void GetAnimsKeyframe(Bone *bone, PlayingAnimation *pAnim, Keyframe **next, Keyframe **prev){

	Animation *anim = pAnim->anim;

	int f;
	for(f = 0; f < anim->nKeyframes[bone->index]; f++){

		if(anim->keyframes[bone->index][f].frame <= pAnim->into){

			*prev = &anim->keyframes[bone->index][f];

		} else {

			*next = &anim->keyframes[bone->index][f];
			break;
		}
	}
}

static Quat GetAnimsBoneRot(Bone *bone, PlayingAnimation *pAnim){

	Keyframe *next = NULL;
	Keyframe *prev = NULL;

	GetAnimsKeyframe(bone, pAnim, &next, &prev);

	if(!prev)
		return pAnim->anim->keyframes[bone->index][0].rot;

	if(!next)
		return prev->rot;

	float slerp = (pAnim->into - prev->frame) / (next->frame - prev->frame);

	return Math_Slerp(prev->rot, next->rot, slerp);
}

static Vec3 GetAnimsBonePos(Bone *bone, PlayingAnimation *pAnim){

	Keyframe *next = NULL;
	Keyframe *prev = NULL;

	GetAnimsKeyframe(bone, pAnim, &next, &prev);

	if(!prev)
		return pAnim->anim->keyframes[bone->index][0].pos;

	if(!next)
		return prev->pos;

	float lerp = (pAnim->into - prev->frame) / (next->frame - prev->frame);

	return Math_LerpVec3(prev->pos, next->pos, lerp);
}


// i store the displacement for each bone as a quaternion and then i have a recursive function that goes through each 
// bone calculating the force to apply to the angular velocity which is converted to a quaternion and multiplied by 
// the current displacement and set to the current displacement.

// and at the same time that angular velocity quaternions inverse is multiplied by each child bones displacement and set to it. for example
// bone->children[j]->displacement = QuatMult(bone->children[j]->displacement, QuatInv(quat));

// (quat here is calculated from the bones angular velocity)

// then each of the bones children is iterated over and the recursion occurs there.

// to get the force from the displacement i rotate a vector of (0,1,0) by the displacement and then take the cross product
// of that and (0,1,0) and use that as the displacement vector in the standard linear spring damper equation.
// since the cross product of the rotated up and the up returns the vector orthogonal to both multiplied by the sine of the
// angle between them it seems to work, so yeah that's why you don't normalize it.

// not sure if this is completely accurate though but it seems to be working.

static void UpdateBoneSpring(Bone *bone, Quat displacement){

	float dt = GetDeltaTime();

	if(bone->spring != 0 && dt > 0 ){

		Vec3 curr = Math_QuatRotate(displacement, (Vec3){0,1,0});

		if(Math_Vec3Dot(curr, (Vec3){0,1,0}) == 1)
			return;

		Vec3 sinaxis = Math_Vec3Cross(curr, (Vec3){0,1,0});

		Vec3 force = Math_Vec3MultFloat(sinaxis, -bone->spring);

		force = Math_Vec3AddVec3(force, Math_Vec3MultFloat(bone->angVel, -bone->damping));

		force = Math_Vec3MultFloat(force, dt);

		bone->angVel = Math_Vec3AddVec3(bone->angVel, force);

		Vec3 vec = Math_Vec3Normalize(bone->angVel);

		float angle = -dt * Math_Vec3Magnitude(bone->angVel);

		Quat quat = Math_Quat(vec, angle);

		bone->rotDisplacement = Math_QuatNormalize(Math_QuatMult(bone->rotDisplacement, quat));

		int j;
		for(j = 0; j < bone->nChildren; j++)
			bone->children[j]->rotDisplacement = Math_QuatMult(bone->children[j]->rotDisplacement, Math_QuatInv(quat));
	}
}

static void BoneUpdateSprings(Bone *bone, Vec4 *matrices){

	UpdateBoneSpring(bone, bone->rotDisplacement);

	Vec3 pos = bone->pos;
	Quat rot = Math_QuatMult(bone->rot, bone->rotDisplacement);

	float matrix[16];

	Math_TranslateMatrix(matrix, pos);
	Math_MatrixFromQuat(rot, bone->absMatrix);
	Math_MatrixMatrixMult(bone->absMatrix, matrix, bone->absMatrix);

	if(bone->parent)
		Math_MatrixMatrixMult(bone->absMatrix, bone->parent->absMatrix, bone->absMatrix); 

	Math_MatrixMatrixMult(matrix, bone->absMatrix, bone->invBindMatrix);

	matrices[(bone->index*3)].x = matrix[0];
	matrices[(bone->index*3)].y = matrix[1];
	matrices[(bone->index*3)].z = matrix[2];
	matrices[(bone->index*3)].w = matrix[3];

	matrices[(bone->index*3)+1].x = matrix[4];
	matrices[(bone->index*3)+1].y = matrix[5];
	matrices[(bone->index*3)+1].z = matrix[6];
	matrices[(bone->index*3)+1].w = matrix[7];

	matrices[(bone->index*3)+2].x = matrix[8];
	matrices[(bone->index*3)+2].y = matrix[9];
	matrices[(bone->index*3)+2].z = matrix[10];
	matrices[(bone->index*3)+2].w = matrix[11];

	int j;
	for(j = 0; j < bone->nChildren; j++)
		BoneUpdateSprings(bone->children[j], matrices);
}

static void BoneUpdate(Bone *bone, PlayingAnimation *anims, int nAnims, Vec4 *matrices){

	Quat rot = (Quat){0,0,0,1};
	Vec3 pos = (Vec3){0,0,0};

	int j;
	float matrix[16];

	for(j = 0; j < nAnims; j++){
		if(anims[j].anim->nKeyframes[bone->index]){
			rot = Math_Slerp(rot, GetAnimsBoneRot(bone, &anims[j]), anims[j].weight);
			pos = Math_LerpVec3(pos, GetAnimsBonePos(bone, &anims[j]), anims[j].weight);
		}
	}

	if(bone->spring != 0){

		if(bone->parent && bone->parent->spring == 0)
			UpdateBoneSpring(bone, Math_QuatMult(bone->parent->rotDisplacement, Math_QuatMult(bone->rotDisplacement, rot)));
		else
			UpdateBoneSpring(bone, Math_QuatMult(bone->rotDisplacement, rot));

		rot = Math_QuatMult(bone->rot, bone->rotDisplacement);
	
	} else {

		// accumulate rotation in rotDisplacement

		bone->rotDisplacement = Math_QuatMult(bone->rotDisplacement, rot);
		
		for(j = 0; j < bone->nChildren; j++){
			if(bone->children[j]->spring == 0)
				bone->children[j]->rotDisplacement = bone->rotDisplacement; // then recursion occurs and line 258 is executed
		}


		rot = Math_QuatMult(bone->rot, rot);
	}

	pos = Math_Vec3AddVec3(bone->pos, Math_QuatRotate(bone->rot, pos));

	Math_TranslateMatrix(matrix, pos);
	Math_MatrixFromQuat(rot, bone->absMatrix);
	Math_MatrixMatrixMult(bone->absMatrix, matrix, bone->absMatrix);

	if(bone->parent)
		Math_MatrixMatrixMult(bone->absMatrix, bone->parent->absMatrix, bone->absMatrix); 

	Math_MatrixMatrixMult(matrix, bone->absMatrix, bone->invBindMatrix);

	matrices[(bone->index*3)].x = matrix[0];
	matrices[(bone->index*3)].y = matrix[1];
	matrices[(bone->index*3)].z = matrix[2];
	matrices[(bone->index*3)].w = matrix[3];

	matrices[(bone->index*3)+1].x = matrix[4];
	matrices[(bone->index*3)+1].y = matrix[5];
	matrices[(bone->index*3)+1].z = matrix[6];
	matrices[(bone->index*3)+1].w = matrix[7];

	matrices[(bone->index*3)+2].x = matrix[8];
	matrices[(bone->index*3)+2].y = matrix[9];
	matrices[(bone->index*3)+2].z = matrix[10];
	matrices[(bone->index*3)+2].w = matrix[11];

	for(j = 0; j < bone->nChildren; j++)
		BoneUpdate(bone->children[j], anims, nAnims, matrices);
}

static void NormalizeAnimWeights(PlayingAnimation *anims, int nAnims){

	float mag = 0;

	int k;
	for(k = 0; k < nAnims; k++)
		mag += anims[k].weight * anims[k].weight; 

	mag = sqrt(mag);

	for(k = 0; k < nAnims; k++)
		anims[k].weight /= mag;		
}

void Skeleton_BlendAnims(PlayingAnimation *anims, int nAnims, float dt){

	int k;
	for(k = 0; k < nAnims; k++){

		if(anims[k].active)
			anims[k].weight = MIN(anims[k].weight + (anims[k].weightSpeed * dt), 1);			
		else
			anims[k].weight = MAX(anims[k].weight - (anims[k].weightSpeed * dt), 0);			
	}

}

void Skeleton_Update(Skeleton *skeleton, PlayingAnimation *anims, int nAnims){

	if(nAnims > 1)
		NormalizeAnimWeights(anims, nAnims);

	BoneUpdate(skeleton->root, anims, nAnims, skeleton->matrices);
}

void Skeleton_UpdateSprings(Skeleton *skeleton){

	BoneUpdateSprings(skeleton->root, skeleton->matrices);
}

static void LoadModel(Model *model, FILE *fp, u16 stride){

	fread(&model->nMaterials, 1, sizeof(int), fp);

	if(model->nMaterials >= MAX_MODEL_MATERIALS){

		LOG(LOG_RED, "FATAL ERROR: NUM MATERIALS IN MODEL ARE MORE THAN MAX ALLOWED MATERIALS\n");
	}

	int k;
	for(k = 0; k < model->nMaterials; k++){
		fread(&model->materials[k].useSSS, 1, sizeof(int), fp); // store tex index for tmp use
		fread(&model->materials[k].texture, 1, sizeof(int), fp); // store tex index for tmp use
		fread(&model->materials[k].normalTexture, 1, sizeof(int), fp); // store tex index for tmp use
		fread(&model->materials[k].specularHardness, 1, sizeof(float), fp);
		fread(&model->materials[k].ambient, 1, sizeof(float), fp);
		fread(&model->materials[k].diffuse, 1, sizeof(Vec4), fp);
		fread(&model->materials[k].specular, 1, sizeof(Vec4), fp);
	}

	u32 textures[MAX_MODEL_MATERIALS];

	int nTextures;
	fread(&nTextures, 1, sizeof(int), fp);

	for(k = 0; k < nTextures; k++){
	
		int w, h, channels;
		fread(&w, 1, sizeof(int), fp);
		fread(&h, 1, sizeof(int), fp);
		fread(&channels, 1, sizeof(int), fp);

	    glGenTextures(1, &textures[k]);
	    glBindTexture(GL_TEXTURE_2D, textures[k]);

	    int size = w * h * channels;

		u8 *data = (u8 *)Memory_StackAlloc(TEMP_STACK, size);
		// Deflate_Read(fp, data, size);
		fread(data, sizeof(u8), size, fp);

		if(channels == 3) channels = GL_RGB;
		else if(channels == 4) channels = GL_RGBA;
		else channels = GL_RED;

	    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, channels, GL_UNSIGNED_BYTE, data);
		Memory_StackPop(TEMP_STACK, 1);

        glGenerateMipmap(GL_TEXTURE_2D);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);//GL_NEAREST_MIPMAP_NEAREST));
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);//GL_NEAREST_MIPMAP_NEAREST));
	    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	    glBindTexture(GL_TEXTURE_2D, 0);
	}

	for(k = 0; k < model->nMaterials; k++){

		if(model->materials[k].texture > 0){
			++model->nTextures;
			model->materials[k].texture = textures[model->materials[k].texture-1];
		}

		if(model->materials[k].normalTexture > 0){
			++model->nNormalTextures;
			model->materials[k].normalTexture = textures[model->materials[k].normalTexture-1];
		}
	}

	int nVerts;
	fread(&nVerts, 1, sizeof(int), fp);

	int size = stride * nVerts;

	u8 *vboData = (u8 *)Memory_StackAlloc(TEMP_STACK, size);

	// Deflate_Read(fp, vboData, size);
	
	fread(vboData, 1, size, fp);

    glBindBuffer(GL_ARRAY_BUFFER, model->vbo);
    glBufferData(GL_ARRAY_BUFFER, size, vboData, GL_STATIC_DRAW);

    Memory_StackPop(TEMP_STACK, 1);

    int totalElements = 0;

	for(k = 0; k < model->nMaterials; k++){

		fread(&model->nElements[k], 1, sizeof(int), fp);

		totalElements += model->nElements[k];

		// Deflate_Read(fp, model->elements[k], sizeof(u16) * model->nElements[k]);
	}

	u32 *elements = (u32 *)Memory_StackAlloc(TEMP_STACK, sizeof(u32) * totalElements);

	fread(elements, totalElements, sizeof(u32), fp);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, totalElements * sizeof(u32), elements, GL_STATIC_DRAW);


	Memory_StackPop(TEMP_STACK, 1);
}

void Model_Load(Model *model, const char *path){

	// u16 stride = sizeof(Vec2) + (sizeof(Vec3) * 4);
	// u16 stride = sizeof(Vec2) + (sizeof(Vec3) * 3);
	u16 stride = sizeof(Vec2) + (sizeof(Vec3) * 2) + sizeof(Vec4);

    glGenVertexArrays(1, &model->vao);
    glBindVertexArray(model->vao);

    glGenBuffers(1, &model->ebo);
    glGenBuffers(1, &model->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, model->vbo);

    glEnableVertexAttribArray(POS_LOC);
    glVertexAttribPointer(POS_LOC, 3, GL_FLOAT, GL_FALSE, stride, 0);

    void *offset = (void *)sizeof(Vec3);

    glEnableVertexAttribArray(UV_LOC);
    glVertexAttribPointer(UV_LOC, 2, GL_FLOAT, GL_FALSE, stride, (void*)offset);

	offset += sizeof(Vec2);

    glEnableVertexAttribArray(NORM_LOC);
    glVertexAttribPointer(NORM_LOC, 3, GL_FLOAT, GL_FALSE, stride, (void*)offset);

	offset += sizeof(Vec3);

    glEnableVertexAttribArray(TANGENT_LOC);
    glVertexAttribPointer(TANGENT_LOC, 4, GL_FLOAT, GL_FALSE, stride, (void*)offset);
	
	// offset += sizeof(Vec4);

 //    glEnableVertexAttribArray(TANGENT_LOC);
 //    glVertexAttribPointer(TANGENT_LOC, 3, GL_FLOAT, GL_FALSE, stride, (void*)offset);
	
	// offset += sizeof(Vec3);

    // glEnableVertexAttribArray(BITANGENT_LOC);
    // glVertexAttribPointer(BITANGENT_LOC, 3, GL_FLOAT, GL_FALSE, stride, (void*)offset);
	
	// offset += sizeof(Vec3);


	FILE *fp = fopen(path, "rb");

	LoadModel(model, fp, stride);

	fclose(fp);

    glBindVertexArray(0);
}

void Model_DeleteTextures(Model *model){
	
	int k;
	for(k = 0; k < model->nMaterials; k++)
	    glDeleteTextures(1, &model->materials[k].texture);
}

void Model_Free(Model *model){

	Model_DeleteTextures(model);

	glDeleteVertexArrays(1, &model->vao);
	glDeleteBuffers(1, &model->vbo);
	glDeleteBuffers(1, &model->ebo);
}

static int LoadAnimation(Animation *anim, FILE *fp){

	int nBones;
	fread(&nBones, 1, sizeof(int), fp);

	memset(anim, 0, sizeof(Animation));

	int nAllocations = 0;

	int k;
	for(k = 0; k < nBones; k++){

		fread(&anim->nKeyframes[k], 1, sizeof(int), fp);

		if(!anim->nKeyframes[k]) continue;
	
		++nAllocations;

		anim->keyframes[k] = Memory_StackAlloc(MAIN_STACK, sizeof(Keyframe) * anim->nKeyframes[k]);

		memset(anim->keyframes[k], 0, sizeof(Keyframe) * anim->nKeyframes[k]);

		int j;
		for(j = 0; j < anim->nKeyframes[k]; j++){
	
			fread(&anim->keyframes[k][j].frame, 1, sizeof(int), fp);
			anim->keyframes[k][j].boneIndex = k;
			fread(&anim->keyframes[k][j].pos, 1, sizeof(Vec3), fp);
			fread(&anim->keyframes[k][j].rot, 1, sizeof(Quat), fp);

			if(anim->keyframes[k][j].frame > anim->length)
				anim->length = anim->keyframes[k][j].frame;

		}
	}

	return nAllocations;
}

void Animation_Free(Animation anim){

	int nAllocations = 0;

	int k;
	for(k = 0; k < MAX_BONES; k++)
		if(anim.keyframes[k])
			++nAllocations;

	Memory_StackPop(MAIN_STACK, nAllocations);
}

static void InitBone(Skeleton *skeleton, Bone *bone){

	static float matrix[16];
	Math_TranslateMatrix(matrix, bone->pos);
	Math_MatrixFromQuat(bone->rot, bone->absMatrix);
	Math_MatrixMatrixMult(bone->absMatrix, matrix, bone->absMatrix);

	// bone->modelPos = bone->rest;

	if(bone->parent){
		Math_MatrixMatrixMult(bone->absMatrix, bone->parent->absMatrix, bone->absMatrix); 
		// bone->modelPos = Math_Vec3AddVec3(bone->parent->modelPos, bone->modelPos);
	}

	memcpy(bone->invBindMatrix, bone->absMatrix, sizeof(float) * 16);
	Math_InverseMatrix(bone->invBindMatrix);

	bone->rotDisplacement = (Quat){0,0,0,1};

	// bone->modelPos.x = bone->absMatrix[3];
	// bone->modelPos.y = bone->absMatrix[7];
	// bone->modelPos.z = bone->absMatrix[11];

	skeleton->matrices[(bone->index*3)].x = 1;
	skeleton->matrices[(bone->index*3)].y = 0;
	skeleton->matrices[(bone->index*3)].z = 0;
	skeleton->matrices[(bone->index*3)].w = 0;
	skeleton->matrices[(bone->index*3)+1].x = 0;
	skeleton->matrices[(bone->index*3)+1].y = 1;
	skeleton->matrices[(bone->index*3)+1].z = 0;
	skeleton->matrices[(bone->index*3)+1].w = 0;
	skeleton->matrices[(bone->index*3)+2].x = 0;
	skeleton->matrices[(bone->index*3)+2].y = 0;
	skeleton->matrices[(bone->index*3)+2].z = 1;
	skeleton->matrices[(bone->index*3)+2].w = 0;

	int k;
	for(k = 0; k < bone->nChildren; k++)
		InitBone(skeleton, bone->children[k]);
}

static void LoadSkeleton(Skeleton *skeleton, FILE *fp){

	memset(skeleton, 0, sizeof(Skeleton));

	fread(&skeleton->nBones, 1, sizeof(int), fp);


	int k;
	for(k = 0; k < skeleton->nBones; k++){

		int index;
		int parentIndex;

		fread(&parentIndex, 1, sizeof(int), fp);
		fread(&index, 1, sizeof(int), fp);

		Bone *bone = &skeleton->bones[index];

		bone->index = index;

		fread(&bone->pos, 1, sizeof(Vec3), fp);
		fread(&bone->rot, 1, sizeof(Quat), fp);
		fread(&bone->cube, 1, sizeof(Cube), fp);

		if(parentIndex >= 0){

			bone->parent = &skeleton->bones[parentIndex];

			if(bone->parent->nChildren < BONE_MAX_CHILDREN)
				bone->parent->children[bone->parent->nChildren++] = bone;
		
		} else {

			skeleton->root = bone;
		}
	}

	InitBone(skeleton, skeleton->root);
}

void Animation_Load(Animation *animation, const char *path){

	FILE *fp = fopen(path, "rb");

	LoadAnimation(animation, fp);

	fclose(fp);
}

void RiggedModel_Load(Model *model, Skeleton *skeleton, const char *path){

	// u16 stride = sizeof(Vec2) + (sizeof(Vec3) * 4) + (sizeof(Vec4) * 2);
	// u16 stride = sizeof(Vec2) + (sizeof(Vec3) * 3) + (sizeof(Vec4) * 2);
	u16 stride = sizeof(Vec2) + (sizeof(Vec3) * 2) + (sizeof(Vec4) * 3);

    glGenVertexArrays(1, &model->vao);
    glBindVertexArray(model->vao);

    glGenBuffers(1, &model->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->ebo);
    
    glGenBuffers(1, &model->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, model->vbo);

    glEnableVertexAttribArray(POS_LOC);
    glVertexAttribPointer(POS_LOC, 3, GL_FLOAT, GL_FALSE, stride, 0);

    void *offset = (void *)sizeof(Vec3);

    glEnableVertexAttribArray(UV_LOC);
    glVertexAttribPointer(UV_LOC, 2, GL_FLOAT, GL_FALSE, stride, (void*)offset);
	
	offset += sizeof(Vec2);

    glEnableVertexAttribArray(NORM_LOC);
    glVertexAttribPointer(NORM_LOC, 3, GL_FLOAT, GL_FALSE, stride, (void*)offset);
	
	offset += sizeof(Vec3);

    glEnableVertexAttribArray(TANGENT_LOC);
    glVertexAttribPointer(TANGENT_LOC, 4, GL_FLOAT, GL_FALSE, stride, (void*)offset);
	
	offset += sizeof(Vec4);

 //    glEnableVertexAttribArray(TANGENT_LOC);
 //    glVertexAttribPointer(TANGENT_LOC, 3, GL_FLOAT, GL_FALSE, stride, (void*)offset);
	
	// offset += sizeof(Vec3);

 //    glEnableVertexAttribArray(BITANGENT_LOC);
 //    glVertexAttribPointer(BITANGENT_LOC, 3, GL_FLOAT, GL_FALSE, stride, (void*)offset);
	
	// offset += sizeof(Vec3);

    glEnableVertexAttribArray(WEIGHTS_LOC);
    glVertexAttribPointer(WEIGHTS_LOC, 4, GL_FLOAT, GL_FALSE, stride, (void*)offset);
    
    offset += sizeof(Vec4);

    glEnableVertexAttribArray(BONE_INDICES_LOC);
    glVertexAttribPointer(BONE_INDICES_LOC, 4, GL_FLOAT, GL_FALSE, stride, (void*)offset);

	FILE *fp = fopen(path, "rb");

	LoadModel(model, fp, stride);

	LoadSkeleton(skeleton, fp);

	fclose(fp);

    glBindVertexArray(0);
}

void RiggedModel_Free(Model *model){

	Model_Free(model);
}