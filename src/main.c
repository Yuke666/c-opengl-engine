#include <GL/glew.h>
#include <stdio.h>
#include "math.h"
#include "physics.h"
#include "models.h"
#include "model.h"
#include "text.h"
#include "renderer.h"
#include "game.h"
#include "images.h"
#include "particles.h"
#include "shaders.h"
#include "window.h"
#include "log.h"
#include "shader_files.h"
#include "memory.h"
#include "utils.h"

#define MOVE_SPEED 1
#define MOUSE_SENSITIVITY 0.005

static Vec3 vel = (Vec3){0,0,0};
static Renderer renderer;
static float dt;
static float lastTime;
static u8 quit;
static Vec2 rotation = (Vec2){0,0};
static Vec3 camPos = (Vec3){-2,-0.5,-3};
static Object groundObj;
static Object nepObj;
static Animation nepAnim;
static Skeleton nepSkeleton;
static Object dickObj;
static Skeleton dickSkeleton;
static PlayingAnimation nepPlayingAnims[1];
static int mode = 0;
static float speed = 40;
static Vec2 dickVel = (Vec2){0,0};
static RigidBody rigidBody;

static void InitBones(void){

	// Bone 0
	// Bone.001 1
	// Bone.002 2
	// Bone.003 3
	// Bone.008 4
	// Bone.009 5
	// Bone.010 6
	// Bone.004 7
	// Bone.005 8
	// Bone.006 9
	// Bone.007 10
	// Bone.011 11
	// Bone.012 12
	// Bone.013 13
	// Bone.014 14
	// Bone.015 15
	// Bone.016 16
	// Bone.017 17
	// Bone.018 18
	// Bone.019 19
	// Bone.028 20
	// Bone.029 21
	// Bone.030 22
	// Bone.031 23
	// Bone.032 24
	// Bone.033 25
	// Bone.034 26
	// Bone.035 27
	// Bone.020 28
	// Bone.022 29
	// Bone.024 30
	// Bone.026 31
	// Bone.021 32
	// Bone.023 33
	// Bone.025 34
	// Bone.027 35

	dickSkeleton.bones[0].rotDisplacement = Math_Quat(Math_Vec3Normalize((Vec3){0.5,0,0.5}), PI/4);

	int k;
	for(k = 0; k < dickSkeleton.nBones; k++){
		dickSkeleton.bones[k].spring = 0.05;
		dickSkeleton.bones[k].damping = 0.1;
	}

	nepSkeleton.bones[4].rotDisplacement = Math_Quat(Math_Vec3Normalize((Vec3){0,0,1}), -0.1);
	nepSkeleton.bones[8].rotDisplacement = Math_Quat(Math_Vec3Normalize((Vec3){0,0,1}), 0.1);

	nepSkeleton.bones[4].spring = 0.05;
	nepSkeleton.bones[4].damping = 0.1;
	nepSkeleton.bones[5].spring = 0.05;
	nepSkeleton.bones[5].damping = 0.1;
	nepSkeleton.bones[6].spring = 0.05;
	nepSkeleton.bones[6].damping = 0.1;
	
	nepSkeleton.bones[8].spring = 0.05;
	nepSkeleton.bones[8].damping = 0.1;
	nepSkeleton.bones[9].spring = 0.05;
	nepSkeleton.bones[9].damping = 0.1;
	nepSkeleton.bones[10].spring = 0.05;
	nepSkeleton.bones[10].damping = 0.1;

	nepSkeleton.bones[26].spring = 0.05;
	nepSkeleton.bones[26].damping = 0.1;
	nepSkeleton.bones[27].spring = 0.05;
	nepSkeleton.bones[27].damping = 0.1;
	nepSkeleton.bones[23].spring = 0.05;
	nepSkeleton.bones[23].damping = 0.1;
	nepSkeleton.bones[24].spring = 0.05;
	nepSkeleton.bones[24].damping = 0.1;

}

static void Event(SDL_Event ev){

	if(ev.type == SDL_QUIT || (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE)){

		quit = 1;
	}

	if(ev.type == SDL_MOUSEMOTION){

		rotation.x += MOUSE_SENSITIVITY * ev.motion.xrel;
		rotation.y += MOUSE_SENSITIVITY * ev.motion.yrel;

        while(rotation.x > PI*2) rotation.x -= PI*2;
        while(rotation.x < 0) rotation.x += PI*2;

        if(rotation.y < -PI/2.2) rotation.y = -PI/2.2;
        if(rotation.y > PI/2) rotation.y = PI/2;
    }


	if(ev.type == SDL_QUIT || (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE)){

		quit = 1;
	}

	
	if(ev.type == SDL_KEYUP){

		if(ev.key.keysym.sym == SDLK_w)
			vel.z = 0;
		if(ev.key.keysym.sym == SDLK_s)
			vel.z = 0;
		if(ev.key.keysym.sym == SDLK_a)
			vel.x = 0;
		if(ev.key.keysym.sym == SDLK_d)
			vel.x = 0;
		if(ev.key.keysym.sym == SDLK_u && dickVel.x > 0)
			dickVel.x = 0;
		if(ev.key.keysym.sym == SDLK_j && dickVel.x < 0)
			dickVel.x = 0;
		if(ev.key.keysym.sym == SDLK_h && dickVel.y > 0)
			dickVel.y = 0;
		if(ev.key.keysym.sym == SDLK_k && dickVel.y < 0)
			dickVel.y = 0;
	}

	if(ev.type == SDL_KEYDOWN){

		if(ev.key.keysym.sym == SDLK_m)
			Renderer_SetMode((mode = (mode+1) % 5));
		if(ev.key.keysym.sym == SDLK_w)
			vel.z = 2.0f/speed;
		if(ev.key.keysym.sym == SDLK_s)
			vel.z = -2.0f/speed;
		if(ev.key.keysym.sym == SDLK_a)
			vel.x = 2.0f/speed;
		if(ev.key.keysym.sym == SDLK_d)
			vel.x = -2.0f/speed;
		if(ev.key.keysym.sym == SDLK_u)
			dickVel.x = 2/speed;
		if(ev.key.keysym.sym == SDLK_j)
			dickVel.x = -2/speed;
		if(ev.key.keysym.sym == SDLK_h)
			dickVel.y = 2/speed;
		if(ev.key.keysym.sym == SDLK_k)
			dickVel.y = -2/speed;

		if(ev.key.keysym.sym == SDLK_RETURN){

			InitBones();

		}
	}
}

static void Render(void){

	float model[16], tmp[16];
	Math_RotateMatrix(model, (Vec3){rotation.y, -rotation.x, 0});

	Vec3 forward = Math_MatrixMult3((Vec3){0,0,1}, model);

	camPos = Math_Vec3AddVec3(camPos, Math_MatrixMult3(Math_Vec3MultFloat(vel, dt), model));

	Math_LookAt(renderer.view, camPos, Math_Vec3AddVec3(camPos, forward), (Vec3){0,1,0});

	// dickSkeleton.bones[0].rotDisplacement = Math_QuatMult(dickSkeleton.bones[0].rotDisplacement, 
	// 	Math_Quat(Math_Vec3Normalize((Vec3){1,0,0}), dickVel.x * dt));

	// dickSkeleton.bones[0].rotDisplacement = Math_QuatMult(dickSkeleton.bones[0].rotDisplacement, 
	// 	Math_Quat(Math_Vec3Normalize((Vec3){0,0,1}), dickVel.y * dt));

	Math_ScalingMatrixXYZ(tmp, 0.25, 0.25, 0.25);
	Math_TranslateMatrix(model, (Vec3){-2,-1,-2});
	Math_MatrixMatrixMult(model, model, tmp);

	memcpy(groundObj.matrix, model, sizeof(model));
	Math_InvTranspose(groundObj.matrixInvTrans, model);

	glPolygonOffset(2, -1);

	// Math_ScalingMatrixXYZ(tmp, 0.03, 0.03, 0.03);
	// Math_TranslateMatrix(model, (Vec3){-2.2,-0.9,-2});
	// Math_MatrixMatrixMult(model, model, tmp);
	// Math_RotateMatrix(tmp, (Vec3){0,PI,0});
	// Math_MatrixMatrixMult(model, model, tmp);

	// memcpy(nepObj.matrix, model, sizeof(model));
	// Math_InvTranspose(nepObj.matrixInvTrans, model);


	// Math_TranslateMatrix(model, (Vec3){-1.8,-0.9,-2});
	Math_TranslateMatrix(model, (Vec3){-2,-0.9,-2});
	Math_RotateMatrix(tmp, (Vec3){0,-PI/2,0});

	// tmp[0] = rigidBody.orientation[0];
	// tmp[1] = rigidBody.orientation[1];
	// tmp[2] = rigidBody.orientation[2];
	// tmp[3] = 0;
	// tmp[4] = rigidBody.orientation[3];
	// tmp[5] = rigidBody.orientation[4];
	// tmp[6] = rigidBody.orientation[5];
	// tmp[7] = 0;
	// tmp[8] = rigidBody.orientation[6];
	// tmp[9] = rigidBody.orientation[7];
	// tmp[10] = rigidBody.orientation[8];
	// tmp[11] = 0;
	// tmp[12] = 0;
	// tmp[13] = 0;
	// tmp[14] = 0;
	// tmp[15] = 1;

	Math_MatrixMatrixMult(model, model, tmp);

	Math_ScalingMatrixXYZ(tmp, 0.3, 0.3, 0.3);
	Math_MatrixMatrixMult(model, model, tmp);

	memcpy(dickObj.matrix, model, sizeof(model));
	Math_InvTranspose(dickObj.matrixInvTrans, model);

	// nepSkeleton.bones[0].angVel = Math_Vec3AddVec3(nepSkeleton.bones[0].angVel, Math_Vec3MultFloat(vel, dt));
	// nepSkeleton.bones[0].rotDisplacement = Math_QuatMult(nepSkeleton.bones[0].rotDisplacement, Math_Quat(vel, dt));


    Renderer_AddObjectToFrame(&renderer, &groundObj);
    // Renderer_AddObjectToFrame(&renderer, &nepObj);
    Renderer_AddObjectToFrame(&renderer, &dickObj);

    ShadowSource source;

    source.pos = (Vec3){-2,0,-3};
    source.forward = Math_Vec3Normalize((Vec3){0,-0.5,1});
    source.up = (Vec3){0,1,0};
	source.fov = CAMERA_FOV;

	Renderer_AddShadowCasterToFrame(&renderer, source, 0.4);

    renderer.lights[0].pos = (Vec3){-2,0.2,-2.6};
    renderer.lights[0].global = 0;
    renderer.lights[0].ambient = 0;
    renderer.lights[0].color = (Vec3){1,1,0.5};
    renderer.lights[0].constant = 0;
    renderer.lights[0].linear = 0;
    renderer.lights[0].quadratic = 1;


	renderer.nLights = 1;

    Renderer_Render(&renderer);

	// Physics_Integrate(&rigidBody, dt);

	// glEnable(GL_BLEND);
	// glBlendFunc(GL_ONE, GL_ONE);
	// glDepthMask(GL_FALSE);

	// Particle particles[2];

	// particles[0].createTime = Window_GetTicks();
	// particles[0].lifeTime = 10000;
	// particles[0].pos = (Vec3){-2.6, -0.35,-0.1};
	// particles[0].size = (Vec2){0.8,0.8};
	// particles[0].color = (Vec4){1, 1, 0.2, 1};

	// particles[1].createTime = Window_GetTicks();
	// particles[1].lifeTime = 10000;
	// particles[1].pos = (Vec3){-3.55, -0.35,-0.1};
	// particles[1].size = (Vec2){0.8,0.8};
	// particles[1].color = (Vec4){1, 1, 0.2, 1};

	// Particles_DrawParticles(headlightImage, particles, 2, 0, renderer.camForward, renderer.camPos, renderer.ndPrepass.depthTexture);
	// // Particles_DrawBillboard(dannyImage, (Vec3){-2,1.5,-1}, (Vec2){1,1}, (Vec4){1,1,1,1}, (Rect2D){0,0,1,1});

	// glDisable(GL_BLEND);
	// glDepthMask(GL_TRUE);
}

static void Update(void){

	nepPlayingAnims[0].into += dt;

	if(nepPlayingAnims[0].into >= nepAnim.length + 30)
		nepPlayingAnims[0].into = 0;


    // Skeleton_UpdateSprings(&nepSkeleton);
    Skeleton_UpdateSprings(&dickSkeleton);
    Skeleton_Update(&nepSkeleton, nepPlayingAnims, 1);
    // Skeleton_Update(&dickSkeleton, NULL, 0);
}

float GetDeltaTime(void){

	return dt;
}

static void Loop(void){

	lastTime = 0;

	quit = 0;

	float lastSecond = Window_GetTicks()/1000.0;

	int nFrames = 0;

	while(!quit){

		Window_PollEvent(Event);
		
		float currTime = Window_GetTicks()/1000.0;

		dt = (currTime - lastTime) * speed;

		if(lastTime == 0) dt = 0;

		lastTime = currTime;

		++nFrames;

		if(currTime - lastSecond > 1){
			printf("%f\n", ((currTime - lastSecond)/(float)nFrames) * 1000);
            printf("%i fps\n", nFrames );
			lastSecond = currTime;
			nFrames = 0;
		}

		Update();

		Render();

		Window_Swap();
	}
}

// need better memory handling for models.. really everything for that matter.


static void Init(void){

	Window_Open();

	Memory_Init((0x01 << 20) * 64);

	Shaders_CompileAll();

	Text_Init();

	Renderer_Init(&renderer, WINDOW_INIT_WIDTH, WINDOW_INIT_HEIGHT);

    SDL_SetRelativeMouseMode(1);

	Particles_Init();

	glPixelStorei(GL_UNPACK_ALIGNMENT,1);

	// Utils_LoadImage(&dannyImage, IMAGE_DANNY, GL_LINEAR, 3);

	// Image floorImage;
	// Utils_LoadImage(&floorImage, IMAGE_FLOOR, GL_LINEAR, 3);

	memset(&groundObj, 0, sizeof(groundObj));
	groundObj.occluder = 1;
    Model_Load(&groundObj.model, MODEL_LEVEL_HALO);

	memset(&nepObj, 0, sizeof(nepObj));
	nepObj.occluder = 1;
	nepObj.skeleton = &nepSkeleton;

	memset(&dickObj, 0, sizeof(dickObj));
	dickObj.occluder = 1;
	dickObj.skeleton = &dickSkeleton;

    RiggedModel_Load(&nepObj.model, &nepSkeleton, MODEL_REI);
    Animation_Load(&nepAnim, ANIMATION_REI_ARMATUREACTION);
    
    RiggedModel_Load(&dickObj.model, &dickSkeleton, MODEL_DICK);


    InitBones();

    nepPlayingAnims[0] = (PlayingAnimation){
    	.active = 1,
    	.weight = 1,
    	.into = 0,
    	.anim = &nepAnim,
    };


 //    memset(&rigidBody, 0, sizeof(RigidBody));
    
 //    rigidBody.particles[0].pos = (Vec3){-1, -1, -1};
 //    rigidBody.particles[1].pos = (Vec3){ 1, -1, -1};
 //    rigidBody.particles[2].pos = (Vec3){-1,  1, -1};
 //    rigidBody.particles[3].pos = (Vec3){ 1,  1, -1};
 //    rigidBody.particles[4].pos = (Vec3){-1, -1,  1};
 //    rigidBody.particles[5].pos = (Vec3){ 1, -1,  1};
 //    rigidBody.particles[6].pos = (Vec3){-1,  1,  1};
 //    rigidBody.particles[7].pos = (Vec3){ 1,  1,  1};
    
 //    rigidBody.particles[0].mass = 10;
 //    rigidBody.particles[1].mass = 10;
 //    rigidBody.particles[2].mass = 10;
 //    rigidBody.particles[3].mass = 10;
 //    rigidBody.particles[4].mass = 10;
 //    rigidBody.particles[5].mass = 10;
 //    rigidBody.particles[6].mass = 10;
 //    rigidBody.particles[7].mass = 10;

 //    rigidBody.torque = (Vec3){0.1,0,0};

 //    rigidBody.nParticles = 8;

	// Physics_InitRigidBody(&rigidBody);


}

static void Exit(void){
    Animation_Free(nepAnim);
	RiggedModel_Free(&nepObj.model);
	Renderer_Close(&renderer);
	Model_Free(&groundObj.model);
	Particles_Close();
	Text_Close();
	Memory_Close();
	Shaders_DestroyAll();
	Window_Close();
}

int main(){

	Init();
	Loop();
	Exit();

	return 0;
}