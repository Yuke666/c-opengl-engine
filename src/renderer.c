#include <GL/glew.h>
#include "renderer.h"
#include "images.h"
#include "math.h"
#include "log.h"
#include "shaders.h"
#include "game.h"
#include "window.h"
#include "model.h"
#include "shader_files.h"

#define NUM_ICOSPHERE_ELEMENTS 240

static const Vec3 icoSphereVerts[42] = {
	{0.000000, 0.000000, -1.000000},
	{0.425323, -0.309011, -0.850654},
	{-0.162456, -0.499995, -0.850654},
	{0.723607, -0.525725, -0.447220},
	{0.850648, 0.000000, -0.525736},
	{-0.525730, 0.000000, -0.850652},
	{-0.162456, 0.499995, -0.850654},
	{0.425323, 0.309011, -0.850654},
	{0.951058, -0.309013, 0.000000},
	{-0.276388, -0.850649, -0.447220},
	{0.262869, -0.809012, -0.525738},
	{0.000000, -1.000000, 0.000000},
	{-0.894426, 0.000000, -0.447216},
	{-0.688189, -0.499997, -0.525736},
	{-0.951058, -0.309013, 0.000000},
	{-0.276388, 0.850649, -0.447220},
	{-0.688189, 0.499997, -0.525736},
	{-0.587786, 0.809017, 0.000000},
	{0.723607, 0.525725, -0.447220},
	{0.262869, 0.809012, -0.525738},
	{0.587786, 0.809017, 0.000000},
	{0.587786, -0.809017, 0.000000},
	{-0.587786, -0.809017, 0.000000},
	{-0.951058, 0.309013, 0.000000},
	{0.000000, 1.000000, 0.000000},
	{0.951058, 0.309013, 0.000000},
	{0.276388, -0.850649, 0.447220},
	{0.688189, -0.499997, 0.525736},
	{0.162456, -0.499995, 0.850654},
	{-0.723607, -0.525725, 0.447220},
	{-0.262869, -0.809012, 0.525738},
	{-0.425323, -0.309011, 0.850654},
	{-0.723607, 0.525725, 0.447220},
	{-0.850648, 0.000000, 0.525736},
	{-0.425323, 0.309011, 0.850654},
	{0.276388, 0.850649, 0.447220},
	{-0.262869, 0.809012, 0.525738},
	{0.162456, 0.499995, 0.850654},
	{0.894426, 0.000000, 0.447216},
	{0.688189, 0.499997, 0.525736},
	{0.525730, 0.000000, 0.850652},
	{0.000000, 0.000000, 1.000000},
};

static const u16 icoSphereElements[240] = {
	0,1,2,3,1,4,0,2,5,0,5,6,0,6,7,3,4,8,9,10,11,12,13,14,15,16,17,18,19,20,3,8,21,9,11,22,12,14,23,15,17,24,18,20,25,26,
	27,28,29,30,31,32,33,34,35,36,37,38,39,40,40,37,41,40,39,37,39,35,37,37,34,41,37,36,34,36,32,34,34,31,41,34,33,31,33,
	29,31,31,28,41,31,30,28,30,26,28,28,40,41,28,27,40,27,38,40,25,39,38,25,20,39,20,35,39,24,36,35,24,17,36,17,32,36,23,
	33,32,23,14,33,14,29,33,22,30,29,22,11,30,11,26,30,21,27,26,21,8,27,8,38,27,20,24,35,20,19,24,19,15,24,17,23,32,17,16,
	23,16,12,23,14,22,29,14,13,22,13,9,22,11,21,26,11,10,21,10,3,21,8,25,38,8,4,25,4,18,25,7,19,18,7,6,19,6,15,19,6,16,15,
	6,5,16,5,12,16,5,13,12,5,2,13,2,9,13,4,7,18,4,1,7,1,0,7,2,10,9,2,1,10,1,3,10
};

static void RenderModel(Renderer *renderer, Object *obj);
static void RenderModelPassthrough(Object *obj);
static void RenderRiggedModelPassthrough(Object *obj);
static void RenderRiggedModel(Renderer *renderer, Object *obj);
static void UpdateMatrices(Renderer *renderer);
static void UpdateMatrixUniforms(Renderer *renderer, float *projView, float *view);

static void RenderQuad(Renderer *renderer){

	glBindVertexArray(renderer->quadVao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

static void NormalDepthPrepass_Close(NormalDepthPrepass *ndPrepass){
	glDeleteTextures(1, &ndPrepass->depthTexture);
	glDeleteTextures(1, &ndPrepass->normalTexture);
	glDeleteFramebuffers(1, &ndPrepass->framebuffer);
}

static void ShadowRenderer_Close(ShadowRenderer *sRenderer){
	glDeleteTextures(1, &sRenderer->depthTexture);
	glDeleteTextures(1, &sRenderer->shadowMaskTexture);
	glDeleteFramebuffers(1, &sRenderer->maskFramebuffer);
	glDeleteFramebuffers(1, &sRenderer->depthFramebuffer);
}

static void GenerateColorTex(int w, int h, GLuint *tex, int filterType, int internalFormat, int format, int type){
	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, *tex);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, type, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterType);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterType);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
}

static void GenerateDepthTex(int w, int h, GLuint *tex, int internalFormat, int format, int type, int filterType){
	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, *tex);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, type, 0);
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterType);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterType);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
}

static void ShadowRenderer_RenderMasks(Renderer *renderer, int depthTexture){

	ShadowRenderer *sRenderer = &renderer->sRenderer;

	glBindFramebuffer(GL_FRAMEBUFFER, sRenderer->maskFramebuffer);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sRenderer->depthTexture);	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthTexture);	


	glUseProgram(Shaders_GetProgram(PROGRAM_shadow_mask));
	glUniform4fv(Shaders_GetFUniformLoc(PROGRAM_shadow_mask, FUNIFORM_darknesses), 1, sRenderer->darknesses);
	glUniform1i(Shaders_GetFUniformLoc(PROGRAM_shadow_mask, FUNIFORM_nSources), sRenderer->nSources);
	glUniformMatrix4fv(Shaders_GetFUniformLoc(PROGRAM_shadow_mask, FUNIFORM_invProjView), 1, GL_TRUE, renderer->invProjView);
	glUniformMatrix4fv(Shaders_GetFUniformLoc(PROGRAM_shadow_mask, FUNIFORM_lightMatrices), 
		MAX_SHADOW_SOURCES, GL_TRUE, sRenderer->matrices);

	RenderQuad(renderer);
}

static void LightPrepass_Resize(LightPrepass *lPrepass, int resX, int resY){

	glBindTexture(GL_TEXTURE_2D, lPrepass->diffuseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, resX, resY, 0, GL_RGB, GL_FLOAT, 0);
	glBindTexture(GL_TEXTURE_2D, lPrepass->specularTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, resX, resY, 0, GL_RGB, GL_FLOAT, 0);
}

static void LightPrepass_Init(LightPrepass *lPrepass, int resX, int resY){

	glGenFramebuffers(1,&lPrepass->framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, lPrepass->framebuffer);    

	GenerateColorTex(resX, resY, &lPrepass->diffuseTexture, GL_LINEAR, GL_RGB16F, GL_RGB, GL_FLOAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lPrepass->diffuseTexture, 0);
	GenerateColorTex(resX, resY, &lPrepass->specularTexture, GL_LINEAR, GL_RGB16F, GL_RGB, GL_FLOAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, lPrepass->specularTexture, 0);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		LOG(LOG_RED, "LightPrepass: Error creating framebuffer.\n");

	glUseProgram(Shaders_GetProgram(PROGRAM_light_prepass));
	glUniform1i(Shaders_GetFUniformLoc(PROGRAM_light_prepass, FUNIFORM_normalTexture), 1);
	glUniform1i(Shaders_GetFUniformLoc(PROGRAM_light_prepass, FUNIFORM_depthTexture), 0);
}

static void LightPrepass_Close(LightPrepass *lPrepass){
	glDeleteTextures(1, &lPrepass->diffuseTexture);
	glDeleteTextures(1, &lPrepass->specularTexture);
	glDeleteFramebuffers(1, &lPrepass->framebuffer);
}

static void FXAA_Render(Renderer *renderer){

	glUseProgram(Shaders_GetProgram(PROGRAM_fxaa));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderer->ldrTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, renderer->ndPrepass.depthTexture);

	RenderQuad(renderer);
}

static void LightPrepass_Render(Renderer *renderer){

	glBindFramebuffer(GL_FRAMEBUFFER, renderer->lPrepass.framebuffer);

	glDrawBuffers(2, (GLuint[]){GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1});
	
	glClearColor(0,0,0,1);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(Shaders_GetProgram(PROGRAM_light_prepass));

	glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_light_prepass, VUNIFORM_projView), 1, GL_TRUE, renderer->projView);
	glUniformMatrix4fv(Shaders_GetFUniformLoc(PROGRAM_light_prepass, FUNIFORM_invProj), 1, GL_TRUE, renderer->invProj);

	glBindVertexArray(renderer->sphereVao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderer->ndPrepass.depthTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, renderer->ndPrepass.normalTexture);

	// batch lights

	int k;
	for(k = 0; k < renderer->nLights; k++){

		Light *light = &renderer->lights[k];

		Vec4 vcenter = Math_MatrixMult4((Vec4){light->pos.x,light->pos.y,light->pos.z,1}, renderer->view);

		vcenter.x /= vcenter.w;
		vcenter.y /= vcenter.w;
		vcenter.z /= vcenter.w;

		float lightMax = MAX(MAX(light->color.x, light->color.y), light->color.z);
		float radius = (-light->linear + sqrt(light->linear * light->linear - 4 * 
			light->quadratic * (light->constant - (256 / 5.0) * lightMax))) / (2 * light->quadratic);

		glUniform1f(Shaders_GetVUniformLoc(PROGRAM_light_prepass, VUNIFORM_radius), radius);

		glUniform3fv(Shaders_GetFUniformLoc(PROGRAM_light_prepass, FUNIFORM_lightCenter), 1, &vcenter.x);
		glUniform3fv(Shaders_GetFUniformLoc(PROGRAM_light_prepass, FUNIFORM_color), 1, &light->color.x);
		glUniform1f(Shaders_GetFUniformLoc(PROGRAM_light_prepass, FUNIFORM_lightConstant), light->constant);
		glUniform1f(Shaders_GetFUniformLoc(PROGRAM_light_prepass, FUNIFORM_ambient), light->ambient);
		glUniform1f(Shaders_GetFUniformLoc(PROGRAM_light_prepass, FUNIFORM_lightQuadratic), light->quadratic);
		glUniform1f(Shaders_GetFUniformLoc(PROGRAM_light_prepass, FUNIFORM_lightLinear), light->linear);


		if(!light->global){

			float dist = Math_Vec3Magnitude((Vec3){vcenter.x, vcenter.y, vcenter.z});

			float near = renderer->near+1;

			if(dist < radius+near){

				glUniform3fv(Shaders_GetVUniformLoc(PROGRAM_light_prepass, VUNIFORM_center), 1, &renderer->camPos.x);
				glUniform1f(Shaders_GetVUniformLoc(PROGRAM_light_prepass, VUNIFORM_radius), near);

				glCullFace(GL_FRONT);
				glDrawElements(GL_TRIANGLES, NUM_ICOSPHERE_ELEMENTS, GL_UNSIGNED_SHORT, NULL);
				glCullFace(GL_BACK);

			} else {

				glUniform3fv(Shaders_GetVUniformLoc(PROGRAM_light_prepass, VUNIFORM_center), 1, &light->pos.x);
				glDrawElements(GL_TRIANGLES, NUM_ICOSPHERE_ELEMENTS, GL_UNSIGNED_SHORT, NULL);
			}
		
		} else {

			glUniform1f(Shaders_GetVUniformLoc(PROGRAM_light_prepass, VUNIFORM_radius), 0);
			RenderQuad(renderer);
		}
	}

	glBindVertexArray(0);
}

static void Bloom_Render(Renderer *renderer){

	BloomRenderer *bRenderer = &renderer->bloomRenderer;

	int bloomResX = renderer->resX * BLOOM_RES;
	int bloomResY = renderer->resY * BLOOM_RES;

	glUseProgram(Shaders_GetProgram(PROGRAM_bright_pass_filter));

	glBindFramebuffer(GL_FRAMEBUFFER, bRenderer->passTwoFramebuffer);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderer->ndPrepass.normalTexture);

	glViewport(0,0,bloomResX,bloomResY);

	RenderQuad(renderer);

	int numPasses = 1;

	int k;
	for(k = 0; k < numPasses; k++){

		glUseProgram(Shaders_GetProgram(PROGRAM_bloom_pass_one));

		glBindFramebuffer(GL_FRAMEBUFFER, bRenderer->passOneFramebuffer);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, bRenderer->passTwoTexture);

		RenderQuad(renderer);

		glUseProgram(Shaders_GetProgram(PROGRAM_bloom_pass_two));

		glBindFramebuffer(GL_FRAMEBUFFER, bRenderer->passTwoFramebuffer);

		glBindTexture(GL_TEXTURE_2D, bRenderer->passOneTexture);

		RenderQuad(renderer);
	}

	glViewport(0,0,renderer->resX,renderer->resY);
}

static void Bloom_Resize(BloomRenderer *bRenderer, int resX, int resY){

	glBindTexture(GL_TEXTURE_2D, bRenderer->passOneTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, resX, resY, 0, GL_RGB, GL_FLOAT, 0);
	glBindTexture(GL_TEXTURE_2D, bRenderer->passTwoTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, resX, resY, 0, GL_RGB, GL_FLOAT, 0);

	glUseProgram(Shaders_GetProgram(PROGRAM_bloom_pass_one));
	glUniform1f(Shaders_GetFUniformLoc(PROGRAM_bloom_pass_one, FUNIFORM_texelSize), 1.0 / (float)resX);
	glUseProgram(Shaders_GetProgram(PROGRAM_bloom_pass_two));
	glUniform1f(Shaders_GetFUniformLoc(PROGRAM_bloom_pass_one, FUNIFORM_texelSize), 1.0 / (float)resY);
}

static void Bloom_Init(BloomRenderer *bRenderer, int resX, int resY){

	glUseProgram(Shaders_GetProgram(PROGRAM_bloom_pass_one));
	glUniform1f(Shaders_GetFUniformLoc(PROGRAM_bloom_pass_one, FUNIFORM_texelSize), 1.0 / (float)resX);
	glUseProgram(Shaders_GetProgram(PROGRAM_bloom_pass_two));
	glUniform1f(Shaders_GetFUniformLoc(PROGRAM_bloom_pass_one, FUNIFORM_texelSize), 1.0 / (float)resY);

	glGenFramebuffers(1,&bRenderer->passOneFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, bRenderer->passOneFramebuffer);

	GenerateColorTex(resX, resY, &bRenderer->passOneTexture, GL_LINEAR, GL_RGB16F, GL_RGB, GL_FLOAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bRenderer->passOneTexture, 0);

	glGenFramebuffers(1,&bRenderer->passTwoFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, bRenderer->passTwoFramebuffer);    

	GenerateColorTex(resX, resY, &bRenderer->passTwoTexture, GL_LINEAR, GL_RGB16F, GL_RGB, GL_FLOAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bRenderer->passTwoTexture, 0);
}

static void Bloom_Close(BloomRenderer *bRenderer){

	glDeleteFramebuffers(1, &bRenderer->passOneFramebuffer);
	glDeleteFramebuffers(1, &bRenderer->passTwoFramebuffer);
	glDeleteTextures(1, &bRenderer->passOneTexture);
	glDeleteTextures(1, &bRenderer->passTwoTexture);
}

static void ShadowRenderer_Resize(ShadowRenderer *renderer, int resX, int resY){

	glBindTexture(GL_TEXTURE_2D, renderer->depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_MAP_WIDTH * MAX_SHADOW_SOURCES, 
		SHADOW_MAP_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, renderer->shadowMaskTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, resX, resY, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
}

static void ShadowRenderer_Init(ShadowRenderer *renderer, int resX, int resY){

	glGenFramebuffers(1,&renderer->depthFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, renderer->depthFramebuffer);    

	GenerateDepthTex(SHADOW_MAP_WIDTH * MAX_SHADOW_SOURCES, SHADOW_MAP_HEIGHT, 
		&renderer->depthTexture, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
	// GenerateDepthTex(resX, resY, &renderer->depthTexture, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, renderer->depthTexture, 0);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		LOG(LOG_RED, "ShadowRenderer: Error creating framebuffer.\n");

	glGenFramebuffers(1,&renderer->maskFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, renderer->maskFramebuffer);    

	GenerateColorTex(resX, resY, &renderer->shadowMaskTexture, GL_LINEAR, GL_RED, GL_RED, GL_UNSIGNED_BYTE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderer->shadowMaskTexture, 0);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		LOG(LOG_RED, "ShadowRenderer: Error creating framebuffer.\n");

	glUseProgram(Shaders_GetProgram(PROGRAM_shadow_mask));
	glUniform1i(Shaders_GetFUniformLoc(PROGRAM_shadow_mask, FUNIFORM_shadowsTexture), 0);
	glUniform1i(Shaders_GetFUniformLoc(PROGRAM_shadow_mask, FUNIFORM_depthTexture), 1);
}

static void NormalDepthPrepass_Resize(NormalDepthPrepass *ndPrepass, int resX, int resY){
	
	glBindTexture(GL_TEXTURE_2D, ndPrepass->depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, resX, resY, 0, GL_RED, GL_FLOAT, 0);
	glBindTexture(GL_TEXTURE_2D, ndPrepass->normalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, resX, resY, 0, GL_RGBA, GL_FLOAT, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, ndPrepass->rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, resX, resY);
}

static void NormalDepthPrepass_Init(NormalDepthPrepass *ndPrepass, int resX, int resY){

	glGenFramebuffers(1,&ndPrepass->framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, ndPrepass->framebuffer);    

	GenerateColorTex(resX, resY, &ndPrepass->normalTexture, GL_NEAREST, GL_RGBA16F, GL_RGBA, GL_FLOAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ndPrepass->normalTexture, 0);

	GenerateColorTex(resX, resY, &ndPrepass->depthTexture, GL_NEAREST, GL_R16F, GL_RED, GL_FLOAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, ndPrepass->depthTexture, 0);

	glGenRenderbuffers(1, &ndPrepass->rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, ndPrepass->rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, resX, resY);
	
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, ndPrepass->rboDepth);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		LOG(LOG_RED, "NormalDepthPrepass: Error creating framebuffer.\n");
}

static void SSAO_Render(Renderer *renderer){

	NormalDepthPrepass *ndPrepass = &renderer->ndPrepass;
	SSAORenderer *ssao = &renderer->ssao;

	glUseProgram(Shaders_GetProgram(PROGRAM_ssao));

	// preform ssao

	glUniform1f(Shaders_GetVUniformLoc(PROGRAM_ssao, VUNIFORM_aspect), renderer->aspectRatio);
	glUniform1f(Shaders_GetVUniformLoc(PROGRAM_ssao, VUNIFORM_tanHalfFov), renderer->tanHalfFov);
	glUniform2f(Shaders_GetFUniformLoc(PROGRAM_ssao, FUNIFORM_texelSize), 1.0f / (float)renderer->resX, 1.0f / (float)renderer->resY);
	glUniformMatrix4fv(Shaders_GetFUniformLoc(PROGRAM_ssao, FUNIFORM_proj), 1, GL_TRUE, renderer->proj);
	// glUniform1f(Shaders_GetFUniformLoc(PROGRAM_ssao, FUNIFORM_invProjWW), renderer->proj[10] / renderer->proj[11]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ndPrepass->depthTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ndPrepass->normalTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, ssao->noiseTexture);

	glBindFramebuffer(GL_FRAMEBUFFER, ssao->ssaoFramebuffer);

	RenderQuad(renderer);

	// blur

	glBindFramebuffer(GL_FRAMEBUFFER, ssao->blurFramebuffer);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ssao->ssaoTexture);

	glUseProgram(Shaders_GetProgram(PROGRAM_ssao_blur_h));
	glUniform1f(Shaders_GetFUniformLoc(PROGRAM_ssao_blur_h, FUNIFORM_blurSize), SSAO_NOISE_SIZE);

	RenderQuad(renderer);

	glUseProgram(Shaders_GetProgram(PROGRAM_ssao_blur_v));
	glUniform1f(Shaders_GetFUniformLoc(PROGRAM_ssao_blur_v, FUNIFORM_blurSize), SSAO_NOISE_SIZE);

	glBindFramebuffer(GL_FRAMEBUFFER, ssao->ssaoFramebuffer);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ssao->blurTexture);

	RenderQuad(renderer);
}

static void SSAO_SetRadius(float radius){
	glUseProgram(Shaders_GetProgram(PROGRAM_ssao));
	glUniform1f(Shaders_GetFUniformLoc(PROGRAM_ssao, FUNIFORM_radius), radius);
}

static void SSAO_Resize(SSAORenderer *ssao,int resX, int resY){

	glUseProgram(Shaders_GetProgram(PROGRAM_ssao));
	glUniform2f(Shaders_GetFUniformLoc(PROGRAM_ssao, FUNIFORM_noiseScale), 
		resX/(float)SSAO_NOISE_SIZE, resY/(float)SSAO_NOISE_SIZE);

	glBindTexture(GL_TEXTURE_2D, ssao->ssaoTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, resX, resY, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, ssao->blurTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, resX, resY, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
}

static void SSAO_Close(SSAORenderer *ssao){

	glDeleteFramebuffers(1, &ssao->ssaoFramebuffer);
	glDeleteFramebuffers(1, &ssao->blurFramebuffer);
	glDeleteTextures(1, &ssao->blurTexture);
	glDeleteTextures(1, &ssao->noiseTexture);
	glDeleteTextures(1, &ssao->ssaoTexture);
}

static void SSAO_Init(SSAORenderer *ssao, int resX, int resY){

	glUseProgram(Shaders_GetProgram(PROGRAM_ssao));
	glUniform1i(Shaders_GetFUniformLoc(PROGRAM_ssao, FUNIFORM_depthTexture), 0);
	glUniform1i(Shaders_GetFUniformLoc(PROGRAM_ssao, FUNIFORM_normalTexture), 1);
	glUniform1i(Shaders_GetFUniformLoc(PROGRAM_ssao, FUNIFORM_randomTexture), 2);

	glUniform2f(Shaders_GetFUniformLoc(PROGRAM_ssao, FUNIFORM_noiseScale),
		resX/(float)SSAO_NOISE_SIZE, resY/(float)SSAO_NOISE_SIZE);

	Vec3 kernel[SSAO_KERNEL_SIZE];

	int k;
	for(k = 0; k < SSAO_KERNEL_SIZE; k++){

		kernel[k].x = ((rand() % 1000) / 500.0f) - 1;
		kernel[k].y = ((rand() % 1000) / 500.0f) - 1;
		kernel[k].z = ((rand() % 1000) / 1000.0f);

		kernel[k] = Math_Vec3Normalize(kernel[k]);

		float scale = k / (float)SSAO_KERNEL_SIZE;

		kernel[k] = Math_Vec3MultFloat(kernel[k], Math_Lerp(0.1f, 1.0f, scale * scale));
	}

	glUniform3fv(Shaders_GetFUniformLoc(PROGRAM_ssao, FUNIFORM_kernel), SSAO_KERNEL_SIZE, &kernel[0].x);

	float vectors[SSAO_NOISE_SIZE * SSAO_NOISE_SIZE * 2];

	for(k = 0; k < SSAO_NOISE_SIZE * SSAO_NOISE_SIZE; k++){

		Vec3 vec;

		vec.x = (((rand() % 1000) / 500.0f) - 1);
		vec.y = (((rand() % 1000) / 500.0f) - 1);
		vec.z = 0;

		vec = Math_Vec3Normalize(vec);

		vectors[(k*2)] = vec.x;
		vectors[(k*2)+1] = vec.y;
	}

	glGenTextures(1,&ssao->noiseTexture);
	glBindTexture(GL_TEXTURE_2D, ssao->noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, SSAO_NOISE_SIZE, SSAO_NOISE_SIZE, 0, GL_RG, GL_FLOAT, vectors);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1,&ssao->ssaoFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, ssao->ssaoFramebuffer);

	GenerateColorTex(resX, resY, &ssao->ssaoTexture, GL_NEAREST, GL_RED, GL_RED, GL_UNSIGNED_BYTE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao->ssaoTexture, 0);

	glGenFramebuffers(1,&ssao->blurFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, ssao->blurFramebuffer);    

	GenerateColorTex(resX, resY, &ssao->blurTexture, GL_NEAREST, GL_RED, GL_RED, GL_UNSIGNED_BYTE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao->blurTexture, 0);

}

static void ToneMappingBloom_Render(Renderer *renderer){

	glUseProgram(Shaders_GetProgram(PROGRAM_tone_mapping_bloom));

	glBindFramebuffer(GL_FRAMEBUFFER, renderer->ldrFramebuffer);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderer->ndPrepass.normalTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, renderer->bloomRenderer.passTwoTexture);

	RenderQuad(renderer);
}

void Renderer_SetMode(int mode){
	
	glUseProgram(Shaders_GetProgram(PROGRAM_textureless_3d));
	glUniform1i(Shaders_GetFUniformLoc(PROGRAM_textureless_3d, FUNIFORM_mode), mode);
	glUseProgram(Shaders_GetProgram(PROGRAM_textureless_skinned));
	glUniform1i(Shaders_GetFUniformLoc(PROGRAM_textureless_skinned, FUNIFORM_mode), mode);
	glUseProgram(Shaders_GetProgram(PROGRAM_standard_3d));
	glUniform1i(Shaders_GetFUniformLoc(PROGRAM_standard_3d, FUNIFORM_mode), mode);
	glUseProgram(Shaders_GetProgram(PROGRAM_skinned));
	glUniform1i(Shaders_GetFUniformLoc(PROGRAM_skinned, FUNIFORM_mode), mode);
}

void Renderer_Resize(Renderer *renderer, int resX, int resY){

	renderer->resX = resX;
	renderer->resY = resY;

	glUseProgram(Shaders_GetProgram(PROGRAM_fxaa));
	glUniform2f(Shaders_GetFUniformLoc(PROGRAM_fxaa, FUNIFORM_texelSize), 1.0 / (float)resX, 1.0 / (float)resY);
	glUniform2f(Shaders_GetVUniformLoc(PROGRAM_fxaa, VUNIFORM_texelSize), 1.0 / (float)resX, 1.0 / (float)resY);

	Bloom_Resize(&renderer->bloomRenderer, resX * BLOOM_RES, resY * BLOOM_RES);

	glBindTexture(GL_TEXTURE_2D, renderer->ldrTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, resX, resY, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	LightPrepass_Resize(&renderer->lPrepass, resX, resY);
	NormalDepthPrepass_Resize(&renderer->ndPrepass, resX, resY);

	// resX *= HALF_RES;
	// resY *= HALF_RES;

	ShadowRenderer_Resize(&renderer->sRenderer, resX, resY);
	SSAO_Resize(&renderer->ssao,resX, resY);

	glUseProgram(Shaders_GetProgram(PROGRAM_ssao_blur_h));
	glUniform1f(Shaders_GetFUniformLoc(PROGRAM_ssao_blur_h, FUNIFORM_texelSize), 1.0 / (float)resX);
	glUseProgram(Shaders_GetProgram(PROGRAM_ssao_blur_v));
	glUniform1f(Shaders_GetFUniformLoc(PROGRAM_ssao_blur_v, FUNIFORM_texelSize), 1.0 / (float)resY);
}

void Renderer_SetPerspective(Renderer *renderer, float fov, float a, float n, float f){

	renderer->tanHalfFov = tan(fov/2);
	renderer->aspectRatio = a;
	renderer->near = n;

	Math_Perspective(renderer->proj, fov, a, n, f);
}

void Renderer_Init(Renderer *renderer, int resX, int resY){

	renderer->resX = resX;
	renderer->resY = resY;

	// init ldr framebuffer

	glGenFramebuffers(1,&renderer->ldrFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, renderer->ldrFramebuffer);

	GenerateColorTex(resX, resY, &renderer->ldrTexture, GL_LINEAR, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderer->ldrTexture, 0);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		LOG(LOG_RED, "Renderer_Init: Error creating framebuffer.\n");

	glUseProgram(Shaders_GetProgram(PROGRAM_fxaa));
	glUniform2f(Shaders_GetFUniformLoc(PROGRAM_fxaa, FUNIFORM_texelSize), 1.0f / (float)resX, 1.0f / (float)resY);
	glUniform2f(Shaders_GetVUniformLoc(PROGRAM_fxaa, VUNIFORM_texelSize), 1.0f / (float)resX, 1.0f / (float)resY);
	glUniform1i(Shaders_GetFUniformLoc(PROGRAM_fxaa, FUNIFORM_tex), 0);
	glUniform1i(Shaders_GetFUniformLoc(PROGRAM_fxaa, FUNIFORM_depthTexture), 1);

	glUseProgram(Shaders_GetProgram(PROGRAM_tone_mapping_bloom));
	glUniform1i(Shaders_GetFUniformLoc(PROGRAM_tone_mapping_bloom, FUNIFORM_tex), 0);
	glUniform1i(Shaders_GetFUniformLoc(PROGRAM_tone_mapping_bloom, FUNIFORM_bloomTexture), 1);

	// init rest

	Bloom_Init(&renderer->bloomRenderer, resX * BLOOM_RES, resY * BLOOM_RES);

	int k;
	for(k = 0; k < 4; k++){
	
		int programs[] = {PROGRAM_standard_3d, PROGRAM_skinned, PROGRAM_textureless_3d, PROGRAM_textureless_skinned};
	
		glUseProgram(Shaders_GetProgram(programs[k]));
		glUniform1i(Shaders_GetFUniformLoc(programs[k], FUNIFORM_tex), 0);
		glUniform1i(Shaders_GetFUniformLoc(programs[k], FUNIFORM_ssao), 1);
		glUniform1i(Shaders_GetFUniformLoc(programs[k], FUNIFORM_lightSpecular), 2);
		glUniform1i(Shaders_GetFUniformLoc(programs[k], FUNIFORM_lightDiffuse), 3);
		glUniform1i(Shaders_GetFUniformLoc(programs[k], FUNIFORM_shadowMask), 4);
	}


	NormalDepthPrepass_Init(&renderer->ndPrepass, resX, resY);
	LightPrepass_Init(&renderer->lPrepass, resX, resY);

	// resX *= HALF_RES;
	// resY *= HALF_RES;

	SSAO_Init(&renderer->ssao, resX, resY);
	ShadowRenderer_Init(&renderer->sRenderer, resX, resY);

	glUseProgram(Shaders_GetProgram(PROGRAM_ssao_blur_h));
	glUniform1f(Shaders_GetFUniformLoc(PROGRAM_ssao_blur_h, FUNIFORM_texelSize), 1.0 / (float)resX);
	glUseProgram(Shaders_GetProgram(PROGRAM_ssao_blur_v));
	glUniform1f(Shaders_GetFUniformLoc(PROGRAM_ssao_blur_v, FUNIFORM_texelSize), 1.0 / (float)resY);

	Renderer_SetPerspective(renderer, CAMERA_FOV, CAMERA_ASPECT, CAMERA_NEAR, CAMERA_FAR);

	memcpy(renderer->invProj, renderer->proj, sizeof(renderer->proj));
	Math_InverseMatrix(renderer->invProj);

	glUseProgram(Shaders_GetProgram(PROGRAM_standard_3d));
	glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_standard_3d, VUNIFORM_model), 1, GL_TRUE, math_Identity);

	glUseProgram(Shaders_GetProgram(PROGRAM_standard_3d_passthrough));
	glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_standard_3d_passthrough, VUNIFORM_model), 1, GL_TRUE, math_Identity);

	glUseProgram(Shaders_GetProgram(PROGRAM_standard_3d_passthrough_normalmap));
	glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_standard_3d_passthrough_normalmap, VUNIFORM_model), 1, GL_TRUE, math_Identity);
	glUniform1i(Shaders_GetFUniformLoc(PROGRAM_standard_3d_passthrough_normalmap, FUNIFORM_tex), 0);

	glUseProgram(Shaders_GetProgram(PROGRAM_skinned_passthrough_normalmap));
	glUniform1i(Shaders_GetFUniformLoc(PROGRAM_skinned_passthrough_normalmap, FUNIFORM_tex), 0);
	glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_skinned_passthrough_normalmap, VUNIFORM_model), 1, GL_TRUE, math_Identity);

	glUseProgram(Shaders_GetProgram(PROGRAM_textureless_3d));
	glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_standard_3d_passthrough, VUNIFORM_model), 1, GL_TRUE, math_Identity);

	glUseProgram(Shaders_GetProgram(PROGRAM_skinned));
	glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_skinned, VUNIFORM_model), 1, GL_TRUE, math_Identity);

	glUseProgram(Shaders_GetProgram(PROGRAM_particles));
	glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_particles, VUNIFORM_model), 1, GL_TRUE, math_Identity);
	glUniform1i(Shaders_GetFUniformLoc(PROGRAM_particles, FUNIFORM_tex), 0);
	glUniform1i(Shaders_GetFUniformLoc(PROGRAM_particles, FUNIFORM_depthTexture), 1);

	glUseProgram(Shaders_GetProgram(PROGRAM_skinned_passthrough));
	glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_skinned_passthrough, VUNIFORM_model), 1, GL_TRUE, math_Identity);

	float orthoProj[16];
	Math_Ortho(orthoProj, 0, renderer->resX, 0, renderer->resY, -10, 10);

	glUseProgram(Shaders_GetProgram(PROGRAM_2d));
	glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_2d, VUNIFORM_projView), 1, GL_TRUE, orthoProj);
	glUseProgram(Shaders_GetProgram(PROGRAM_textureless_2d));
	glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_textureless_2d, VUNIFORM_projView), 1, GL_TRUE, orthoProj);

	// init quad drawing

	glGenVertexArrays(1, &renderer->quadVao);
	glBindVertexArray(renderer->quadVao);

	glGenBuffers(1, &renderer->quadVbo);

	glBindBuffer(GL_ARRAY_BUFFER, renderer->quadVbo);

	Vec2 rectVerts[] = {{-1,-1}, {1,-1}, {1,1}, {1,1}, {-1,1}, {-1,-1}};

    glEnableVertexAttribArray(POS_LOC);
    glVertexAttribPointer(POS_LOC, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2) * 6, &rectVerts[0].x, GL_STATIC_DRAW);
 
	// init sphere drawing

    glGenVertexArrays(1, &renderer->sphereVao);
    glBindVertexArray(renderer->sphereVao);

    glGenBuffers(1, &renderer->sphereVbo);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->sphereVbo);

    glEnableVertexAttribArray(POS_LOC);
    glVertexAttribPointer(POS_LOC, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), 0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(icoSphereVerts), icoSphereVerts, GL_STATIC_DRAW);

    glGenBuffers(1, &renderer->sphereEbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->sphereEbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(icoSphereElements), icoSphereElements, GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void Renderer_Close(Renderer *renderer){

	glDeleteVertexArrays(1, &renderer->quadVao);
	glDeleteBuffers(1, &renderer->quadVbo);

	glDeleteVertexArrays(1, &renderer->sphereVbo);
	glDeleteBuffers(1, &renderer->sphereEbo);
	glDeleteBuffers(1, &renderer->sphereVbo);

	LightPrepass_Close(&renderer->lPrepass);
	NormalDepthPrepass_Close(&renderer->ndPrepass);
	ShadowRenderer_Close(&renderer->sRenderer);
	SSAO_Close(&renderer->ssao);
	Bloom_Close(&renderer->bloomRenderer);
}

void Renderer_AddObjectToFrame(Renderer *renderer, Object *object){
	
	if(renderer->nObjects+1 >= MAX_ON_SCREEN_OBJECTS) return;

	renderer->objects[renderer->nObjects++] = object;
}

void Renderer_AddLightToFrame(Renderer *renderer, Light light){
	
	if(renderer->nLights+1 >= MAX_ON_SCREEN_LIGHTS) return;

	renderer->lights[renderer->nLights++] = light;
}

void Renderer_AddShadowCasterToFrame(Renderer *renderer, ShadowSource source, float darkness){
	
	if(renderer->sRenderer.nSources+1 >= MAX_SHADOW_SOURCES) return;

	renderer->sRenderer.sources[renderer->sRenderer.nSources] = source;
	renderer->sRenderer.darknesses[renderer->sRenderer.nSources] = darkness;

	renderer->sRenderer.nSources++;
}

void Renderer_Render(Renderer *renderer){

	// i should sort geometry front to back first
	// skip small objects
	// remove some of these clears
	// disable depth testing. gl_EQUAL needed

	// need depth only passthrough. will be faster to not have to calculate TBN, especially for rigged meshes

	glClearColor(0,0,0,1);

	int k;

	UpdateMatrices(renderer);

	// shadows

	ShadowRenderer *sRenderer = &renderer->sRenderer;
	NormalDepthPrepass *ndPrepass = &renderer->ndPrepass;

	glBindFramebuffer(GL_FRAMEBUFFER, sRenderer->depthFramebuffer);

	glClear(GL_DEPTH_BUFFER_BIT);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	for(k = 0; k < sRenderer->nSources; k++){
	
		glViewport(k * SHADOW_MAP_WIDTH, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
	
		float view[16];

		float *projView = &sRenderer->matrices[k * 16];
		Vec3 forward = sRenderer->sources[k].forward;
		Vec3 pos = sRenderer->sources[k].pos;
		Vec3 up = sRenderer->sources[k].up;

		Math_Perspective(projView, sRenderer->sources[k].fov, 1, SHADOW_CASTER_NEAR, SHADOW_CASTER_FAR);
		Math_LookAt(view, pos, Math_Vec3AddVec3(pos, forward), up);
		
		Math_MatrixMatrixMult(projView, projView, view);

		UpdateMatrixUniforms(renderer, projView, view);

		int j;
		for(j = 0; j < renderer->nObjects; j++){

			if(!renderer->objects[j]->occluder)
				continue;

			if(renderer->objects[j]->skeleton)
				RenderRiggedModelPassthrough(renderer->objects[j]);
			else
				RenderModelPassthrough(renderer->objects[j]);
		}
	}

	glViewport(0, 0, renderer->resX, renderer->resY);


	UpdateMatrixUniforms(renderer, renderer->projView, renderer->view);

	// end

	glDrawBuffers(2, (GLuint[]){ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 });

	// nd prepass

	glBindFramebuffer(GL_FRAMEBUFFER, ndPrepass->framebuffer);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for(k = 0; k < renderer->nObjects; k++){

		if(renderer->objects[k]->skeleton)
			RenderRiggedModelPassthrough(renderer->objects[k]);
		else
			RenderModelPassthrough(renderer->objects[k]);
	}

	// end

	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);

	// light prepass
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	LightPrepass_Render(renderer);

	glDisable(GL_BLEND);

	// ssao render, replace alpha of specular with ssao
	
	// glViewport(0, 0, renderer->resX * HALF_RES, renderer->resY * HALF_RES);
	glViewport(0, 0, renderer->resX, renderer->resY);


	SSAO_Render(renderer);

	// finally

	ShadowRenderer_RenderMasks(renderer, renderer->ndPrepass.depthTexture);

	// end

	glViewport(0, 0, renderer->resX, renderer->resY);
	
	glBindFramebuffer(GL_FRAMEBUFFER, ndPrepass->framebuffer);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_EQUAL);
	// glDepthMask(GL_FALSE);
	// glDepthFunc(GL_LESS);
	// glDepthMask(GL_TRUE);

	// glClearColor(1,1,1,1);


	// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, renderer->ssao.ssaoTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, renderer->lPrepass.specularTexture);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, renderer->lPrepass.diffuseTexture);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, sRenderer->shadowMaskTexture);

	// final render

	for(k = 0; k < renderer->nObjects; k++){

		if(renderer->objects[k]->skeleton)
			RenderRiggedModel(renderer, renderer->objects[k]);
		else
			RenderModel(renderer, renderer->objects[k]);
	}

	glDepthFunc(GL_LEQUAL);

	glDisable(GL_DEPTH_TEST);
	// glDepthMask(GL_FALSE);

	Bloom_Render(renderer);
	
	ToneMappingBloom_Render(renderer);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	FXAA_Render(renderer);
	
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	// //end

	// glDepthMask(GL_TRUE);
	// glDepthFunc(GL_LESS);

	// do->transparent objects

	// glEnable(GL_BLEND);
 //    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	// for(k = 0; k < renderer->nObjects; k++){

	// 	if(renderer->transparentObjects[k]->skeleton)
	// 		RenderRiggedModel(renderer, renderer->transparentObjects[k]);
	// 	else
	// 		RenderModel(renderer, renderer->transparentObjects[k]);
	// }

	// glDisable(GL_BLEND);

	// glDepthFunc(GL_LESS);
	// glDepthMask(GL_TRUE);

	renderer->nObjects = 0;
	renderer->sRenderer.nSources = 0;
	renderer->nLights = 0;
}

static void UpdateMatrices(Renderer *renderer){
	
	memcpy(renderer->invView, renderer->view, sizeof(renderer->view));
	Math_InverseMatrix(renderer->invView);

	Math_MatrixMatrixMult(renderer->projView, renderer->proj, renderer->view);

	memcpy(renderer->invProjView, renderer->projView, sizeof(renderer->projView));
	Math_InverseMatrix(renderer->invProjView);

	renderer->camPos.x = renderer->invView[3];
	renderer->camPos.y = renderer->invView[7];
	renderer->camPos.z = renderer->invView[11];
	renderer->camForward.x = renderer->view[2];
	renderer->camForward.y = renderer->view[6];
	renderer->camForward.z = renderer->view[10];

	UpdateMatrixUniforms(renderer, renderer->projView, renderer->view);
}

static void UpdateMatrixUniforms(Renderer *renderer, float *projView, float *view){

	glUseProgram(Shaders_GetProgram(PROGRAM_standard_3d));
	glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_standard_3d, VUNIFORM_projView), 1, GL_TRUE, projView);

	glUseProgram(Shaders_GetProgram(PROGRAM_standard_3d_passthrough));
	glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_standard_3d_passthrough, VUNIFORM_projView), 1, GL_TRUE, projView);
	glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_standard_3d_passthrough, VUNIFORM_view), 1, GL_TRUE, view);

	glUseProgram(Shaders_GetProgram(PROGRAM_standard_3d_passthrough_normalmap));
	glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_standard_3d_passthrough_normalmap, VUNIFORM_projView), 1, GL_TRUE, projView);
	glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_standard_3d_passthrough_normalmap, VUNIFORM_view), 1, GL_TRUE, view);

	glUseProgram(Shaders_GetProgram(PROGRAM_textureless_3d));
	glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_textureless_3d, VUNIFORM_projView), 1, GL_TRUE, projView);

	glUseProgram(Shaders_GetProgram(PROGRAM_textureless_skinned));
	glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_textureless_skinned, VUNIFORM_projView), 1, GL_TRUE, projView);

	glUseProgram(Shaders_GetProgram(PROGRAM_skinned));
	glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_skinned, VUNIFORM_projView), 1, GL_TRUE, projView);

	glUseProgram(Shaders_GetProgram(PROGRAM_skinned_passthrough));
	glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_skinned_passthrough, VUNIFORM_projView), 1, GL_TRUE, projView);
	glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_skinned_passthrough, VUNIFORM_view), 1, GL_TRUE, view);

	glUseProgram(Shaders_GetProgram(PROGRAM_skinned_passthrough_normalmap));
	glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_skinned_passthrough_normalmap, VUNIFORM_projView), 1, GL_TRUE, projView);
	glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_skinned_passthrough_normalmap, VUNIFORM_view), 1, GL_TRUE, view);

	glUseProgram(Shaders_GetProgram(PROGRAM_particles));
	glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_particles, VUNIFORM_projView), 1, GL_TRUE, projView);
	glUniformMatrix4fv(Shaders_GetFUniformLoc(PROGRAM_particles, FUNIFORM_proj), 1, GL_TRUE, renderer->proj);

	float inv[16];
	memcpy(inv, view, sizeof(inv));
	Math_InverseMatrix(inv);

	Vec3 camRight = (Vec3){inv[0], inv[4], inv[8]};
	Vec3 camUp = (Vec3){inv[1], inv[5], inv[9]};

	glUniform3fv(Shaders_GetVUniformLoc(PROGRAM_particles, VUNIFORM_camRight), 1, &camRight.x);
	glUniform3fv(Shaders_GetVUniformLoc(PROGRAM_particles, VUNIFORM_camUp), 1, &camUp.x);
}

static void RenderRiggedModel(Renderer *renderer, Object *obj){

	int currProgram = PROGRAM_skinned;

	if(obj->model.nTextures){
	
		glUseProgram(Shaders_GetProgram(currProgram));
		glUniformMatrix4fv(Shaders_GetVUniformLoc(currProgram, VUNIFORM_model), 1, GL_TRUE, obj->matrix);
		glUniform4fv(Shaders_GetVUniformLoc(currProgram, VUNIFORM_bones), obj->skeleton->nBones * 3, &obj->skeleton->matrices[0].x);

		if(obj->model.nMaterials > obj->model.nTextures){

			currProgram = PROGRAM_textureless_skinned;
			glUseProgram(Shaders_GetProgram(currProgram));
			glUniformMatrix4fv(Shaders_GetVUniformLoc(currProgram, VUNIFORM_model), 1, GL_TRUE, obj->matrix);
			glUniform4fv(Shaders_GetVUniformLoc(currProgram, VUNIFORM_bones), obj->skeleton->nBones * 3, &obj->skeleton->matrices[0].x);
		}
	
		glActiveTexture(GL_TEXTURE0);

	} else {

		currProgram = PROGRAM_textureless_skinned;
		glUseProgram(Shaders_GetProgram(currProgram));
		glUniformMatrix4fv(Shaders_GetVUniformLoc(currProgram, VUNIFORM_model), 1, GL_TRUE, obj->matrix);
		glUniform4fv(Shaders_GetVUniformLoc(currProgram, VUNIFORM_bones), obj->skeleton->nBones * 3, &obj->skeleton->matrices[0].x);
	}
	
	glBindVertexArray(obj->model.vao);

	int curr = 0;

	int k;
	for(k = 0; k < obj->model.nMaterials; k++){

		if(!obj->model.materials[k].texture && currProgram != PROGRAM_textureless_skinned){
		
			glUseProgram(Shaders_GetProgram((currProgram = PROGRAM_textureless_skinned)));
		
		} else if(obj->model.materials[k].texture){

			if(currProgram == PROGRAM_textureless_skinned)
				glUseProgram(Shaders_GetProgram((currProgram = PROGRAM_skinned)));

			glBindTexture(GL_TEXTURE_2D, obj->model.materials[k].texture);
		}

		glUniform1f(Shaders_GetFUniformLoc(currProgram, FUNIFORM_lightAmbient), obj->model.materials[k].ambient);
		glUniform4fv(Shaders_GetFUniformLoc(currProgram, FUNIFORM_diffuse), 1, &obj->model.materials[k].diffuse.x);
		glUniform4fv(Shaders_GetFUniformLoc(currProgram, FUNIFORM_specular), 1, &obj->model.materials[k].specular.x);

		glDrawElements(GL_TRIANGLES, obj->model.nElements[k], GL_UNSIGNED_INT, (void *)(curr * sizeof(GLuint)));
		curr += obj->model.nElements[k];
	}

	glBindVertexArray(0);
}

static void RenderRiggedModelPassthrough(Object *obj){

	int currProgram = PROGRAM_skinned_passthrough_normalmap;

	if(obj->model.nNormalTextures){
		
		if(obj->model.nMaterials > obj->model.nNormalTextures){

			glUseProgram(Shaders_GetProgram(PROGRAM_skinned_passthrough));
			glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_skinned_passthrough, VUNIFORM_model), 1, GL_TRUE, obj->matrix);
			glUniformMatrix3fv(Shaders_GetVUniformLoc(PROGRAM_skinned_passthrough, VUNIFORM_modelInvTranspose), 1, GL_TRUE, obj->matrixInvTrans);
			glUniform4fv(Shaders_GetVUniformLoc(PROGRAM_skinned_passthrough, VUNIFORM_bones), obj->skeleton->nBones * 3, &obj->skeleton->matrices[0].x);

			glUseProgram(Shaders_GetProgram(PROGRAM_skinned_passthrough_normalmap));
			glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_skinned_passthrough_normalmap, VUNIFORM_model), 1, GL_TRUE, obj->matrix);
			glUniformMatrix3fv(Shaders_GetVUniformLoc(PROGRAM_skinned_passthrough_normalmap, VUNIFORM_modelInvTranspose), 1, GL_TRUE, obj->matrixInvTrans);
			glUniform4fv(Shaders_GetVUniformLoc(PROGRAM_skinned_passthrough_normalmap, VUNIFORM_bones), obj->skeleton->nBones * 3, &obj->skeleton->matrices[0].x);
			
		} else {

			glUseProgram(Shaders_GetProgram(PROGRAM_skinned_passthrough_normalmap));
			glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_skinned_passthrough_normalmap, VUNIFORM_model), 1, GL_TRUE, obj->matrix);
			glUniformMatrix3fv(Shaders_GetVUniformLoc(PROGRAM_skinned_passthrough_normalmap, VUNIFORM_modelInvTranspose), 1, GL_TRUE, obj->matrixInvTrans);
			glUniform4fv(Shaders_GetVUniformLoc(PROGRAM_skinned_passthrough_normalmap, VUNIFORM_bones), obj->skeleton->nBones * 3, &obj->skeleton->matrices[0].x);
		}

	} else {

		glUseProgram(Shaders_GetProgram(PROGRAM_skinned_passthrough));
		glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_skinned_passthrough, VUNIFORM_model), 1, GL_TRUE, obj->matrix);
		glUniformMatrix3fv(Shaders_GetVUniformLoc(PROGRAM_skinned_passthrough, VUNIFORM_modelInvTranspose), 1, GL_TRUE, obj->matrixInvTrans);
		glUniform4fv(Shaders_GetVUniformLoc(PROGRAM_skinned_passthrough, VUNIFORM_bones), obj->skeleton->nBones * 3, &obj->skeleton->matrices[0].x);

		currProgram = PROGRAM_skinned_passthrough;
	}
	
	glBindVertexArray(obj->model.vao);
	glActiveTexture(GL_TEXTURE0);

	int curr = 0;

	int k;
	for(k = 0; k < obj->model.nMaterials; k++){

		if(obj->model.materials[k].normalTexture <= 0 && currProgram != PROGRAM_skinned_passthrough){
		
			glUseProgram(Shaders_GetProgram((currProgram = PROGRAM_skinned_passthrough)));
		
		} else if(obj->model.materials[k].normalTexture){

			if(currProgram == PROGRAM_skinned_passthrough)
				glUseProgram(Shaders_GetProgram((currProgram = PROGRAM_skinned_passthrough_normalmap)));

			glBindTexture(GL_TEXTURE_2D, obj->model.materials[k].normalTexture);
		}

		glUniform1f(Shaders_GetFUniformLoc(currProgram, FUNIFORM_specularHardness), obj->model.materials[k].specularHardness);

		glDrawElements(GL_TRIANGLES, obj->model.nElements[k], GL_UNSIGNED_INT, (void *)(curr * sizeof(GLuint)));
		curr += obj->model.nElements[k];
	}

	glBindVertexArray(0);
}

static void RenderModelPassthrough(Object *obj){

	int currProgram = PROGRAM_standard_3d_passthrough;

	if(obj->model.nNormalTextures){
		
		if(obj->model.nMaterials > obj->model.nNormalTextures){

			glUseProgram(Shaders_GetProgram(PROGRAM_standard_3d_passthrough_normalmap));
			glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_standard_3d_passthrough_normalmap, VUNIFORM_model), 1, GL_TRUE, obj->matrix);
			glUniformMatrix3fv(Shaders_GetVUniformLoc(PROGRAM_standard_3d_passthrough_normalmap, VUNIFORM_modelInvTranspose), 1, GL_TRUE, obj->matrixInvTrans);

			glUseProgram(Shaders_GetProgram(PROGRAM_standard_3d_passthrough));
			glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_standard_3d_passthrough, VUNIFORM_model), 1, GL_TRUE, obj->matrix);
			glUniformMatrix3fv(Shaders_GetVUniformLoc(PROGRAM_standard_3d_passthrough, VUNIFORM_modelInvTranspose), 1, GL_TRUE, obj->matrixInvTrans);

		} else {

			currProgram = PROGRAM_standard_3d_passthrough_normalmap;
			glUseProgram(Shaders_GetProgram(currProgram));
			glUniformMatrix4fv(Shaders_GetVUniformLoc(currProgram, VUNIFORM_model), 1, GL_TRUE, obj->matrix);
			glUniformMatrix3fv(Shaders_GetVUniformLoc(currProgram, VUNIFORM_modelInvTranspose), 1, GL_TRUE, obj->matrixInvTrans);
		}

		glActiveTexture(GL_TEXTURE0);
	
	} else {

		glUseProgram(Shaders_GetProgram(currProgram));
		glUniformMatrix4fv(Shaders_GetVUniformLoc(currProgram, VUNIFORM_model), 1, GL_TRUE, obj->matrix);
		glUniformMatrix3fv(Shaders_GetVUniformLoc(currProgram, VUNIFORM_modelInvTranspose), 1, GL_TRUE, obj->matrixInvTrans);
	}
	
	glBindVertexArray(obj->model.vao);

	int curr = 0;

	int k;
	for(k = 0; k < obj->model.nMaterials; k++){

		if(obj->model.materials[k].normalTexture <= 0 && currProgram != PROGRAM_standard_3d_passthrough){

			glUseProgram(Shaders_GetProgram((currProgram = PROGRAM_standard_3d_passthrough)));
		
		} else if(obj->model.materials[k].normalTexture){

			if(currProgram == PROGRAM_standard_3d_passthrough)
				glUseProgram(Shaders_GetProgram((currProgram = PROGRAM_standard_3d_passthrough_normalmap)));

			glBindTexture(GL_TEXTURE_2D, obj->model.materials[k].normalTexture);

		}

		glUniform1f(Shaders_GetFUniformLoc(currProgram, FUNIFORM_specularHardness), obj->model.materials[k].specularHardness);

		glDrawElements(GL_TRIANGLES, obj->model.nElements[k], GL_UNSIGNED_INT, (void *)(curr * sizeof(GLuint)));
		curr += obj->model.nElements[k];
	}

	glBindVertexArray(0);
}

static void RenderModel(Renderer *renderer, Object *obj){

	int currProgram = PROGRAM_standard_3d;

	if(obj->model.nTextures){
		
		if(obj->model.nMaterials > obj->model.nTextures){

			glUseProgram(Shaders_GetProgram(PROGRAM_textureless_3d));
			glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_textureless_3d, VUNIFORM_model), 1, GL_TRUE, obj->matrix);

			glUseProgram(Shaders_GetProgram(PROGRAM_standard_3d));
			glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_standard_3d, VUNIFORM_model), 1, GL_TRUE, obj->matrix);

		} else {

			glUseProgram(Shaders_GetProgram(PROGRAM_standard_3d));
			glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_standard_3d, VUNIFORM_model), 1, GL_TRUE, obj->matrix);
		}

		glActiveTexture(GL_TEXTURE0);

	} else {

		glUseProgram(Shaders_GetProgram(PROGRAM_textureless_3d));
		glUniformMatrix4fv(Shaders_GetVUniformLoc(PROGRAM_textureless_3d, VUNIFORM_model), 1, GL_TRUE, obj->matrix);

		currProgram = PROGRAM_textureless_3d;
	}

	
	glBindVertexArray(obj->model.vao);

	int curr = 0;

	int k;
	for(k = 0; k < obj->model.nMaterials; k++){

		if(obj->model.materials[k].texture <= 0 && currProgram != PROGRAM_textureless_3d){

			glUseProgram(Shaders_GetProgram((currProgram = PROGRAM_textureless_3d)));
		
		} else if(obj->model.materials[k].texture){

			if(currProgram == PROGRAM_textureless_3d)
				glUseProgram(Shaders_GetProgram((currProgram = PROGRAM_standard_3d)));

			glBindTexture(GL_TEXTURE_2D, obj->model.materials[k].texture);

		}

		// Vec3 diffuse = obj->model.materials[k].diffuse;
		// printf("%f %f %f\n", diffuse.x, diffuse.y, diffuse.z);

		glUniform1f(Shaders_GetFUniformLoc(currProgram, FUNIFORM_ambient), obj->model.materials[k].ambient);
		glUniform4fv(Shaders_GetFUniformLoc(currProgram, FUNIFORM_diffuse), 1, &obj->model.materials[k].diffuse.x);
		glUniform4fv(Shaders_GetFUniformLoc(currProgram, FUNIFORM_specular), 1, &obj->model.materials[k].specular.x);

		glDrawElements(GL_TRIANGLES, obj->model.nElements[k], GL_UNSIGNED_INT, (void *)(curr * sizeof(GLuint)));
		curr += obj->model.nElements[k];
	}

	glBindVertexArray(0);
}