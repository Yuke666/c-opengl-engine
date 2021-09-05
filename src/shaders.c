#include <GL/glew.h>
#include <stdio.h>
#include "log.h"
#include "shaders.h"
#include "memory.h"

static int LoadShader(char **into, const char *path){

	FILE *fp = fopen(path, "r");

	if(!fp){
        LOGF(LOG_RED, "NOT FOUND: %s\n", path);
        return -1;
	}

	fseek(fp, 0, SEEK_END);

	int len = ftell(fp);

	rewind(fp);

	*into = (char *)Memory_StackAlloc(TEMP_STACK, len+1);

	fread(*into, 1, len, fp);

    (*into)[len] = 0;

	return 1;
}

void Shader_CompileShader(Shader *shader, const char *vPath, const char *fPath){

    memset(shader, 0, sizeof(Shader));

	char *fSource, *vSource;

    if(LoadShader(&vSource, vPath) < 0) return;

    if(LoadShader(&fSource, fPath) < 0){
    	Memory_StackPop(TEMP_STACK, 1);
    	return;
    }

    shader->program = glCreateProgram();
    shader->fShader = glCreateShader(GL_FRAGMENT_SHADER);
    shader->vShader = glCreateShader(GL_VERTEX_SHADER);
    
    const GLchar* glSrc = fSource;
    
    GLint status;

    char buffer[512];

    glShaderSource(shader->fShader,1,&glSrc,NULL);
    glCompileShader(shader->fShader);
    glGetShaderiv(shader->fShader, GL_COMPILE_STATUS, &status);

	Memory_StackPop(TEMP_STACK, 1);

    if(status != GL_TRUE){
		Memory_StackPop(TEMP_STACK, 1);
        glGetShaderInfoLog(shader->fShader,512,NULL,buffer);
        LOGF(LOG_RED, "FSHADER (%s): %s\n", fPath, buffer);
        return;
    }

    glAttachShader(shader->program, shader->fShader);

    glSrc = vSource;
    glShaderSource(shader->vShader,1,&glSrc,NULL);
    glCompileShader(shader->vShader);
    glGetShaderiv(shader->vShader, GL_COMPILE_STATUS, &status);

	Memory_StackPop(TEMP_STACK, 1);

    if(status != GL_TRUE){
        glGetShaderInfoLog(shader->vShader,512,NULL,buffer);
        LOGF(LOG_RED, "VSHADER (%s): %s\n", vPath, buffer);
        return;
    }

    glBindAttribLocation(shader->program, POS_LOC, POS_ATTRIB);
    glBindAttribLocation(shader->program, UV_LOC, UV_ATTRIB);
    glBindAttribLocation(shader->program, NORM_LOC, NORM_ATTRIB);
    glBindAttribLocation(shader->program, TANGENT_LOC, TANGENT_ATTRIB);
    // glBindAttribLocation(shader->program, BITANGENT_LOC, BITANGENT_ATTRIB);
    glBindAttribLocation(shader->program, COLOR_LOC, COLOR_ATTRIB);
    glBindAttribLocation(shader->program, CENTER_LOC, CENTER_ATTRIB);
    glBindAttribLocation(shader->program, WEIGHTS_LOC, WEIGHTS_ATTRIB);
    glBindAttribLocation(shader->program, BONE_INDICES_LOC, BONE_INDICES_ATTRIB);

    glAttachShader(shader->program, shader->vShader);
    glLinkProgram(shader->program);
    glUseProgram(shader->program);

    glUseProgram(0);
}

void Shader_DestroyShader(Shader *shader){
    glDeleteProgram(shader->program);
    glDeleteShader(shader->fShader);
    glDeleteShader(shader->vShader);
}