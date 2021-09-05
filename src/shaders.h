#ifndef SHADERS_DEF
#define SHADERS_DEF

#include "shader_files.h"

#define POS_ATTRIB "pos"
#define UV_ATTRIB "uv"
#define NORM_ATTRIB "norm"
#define TANGENT_ATTRIB "tangent"
// #define BITANGENT_ATTRIB "bitangent"
#define COLOR_ATTRIB "color"
#define CENTER_ATTRIB "center"
#define WEIGHTS_ATTRIB "weights"
#define BONE_INDICES_ATTRIB "boneIndices"

enum {
    POS_LOC = 0,
    UV_LOC,
    WEIGHTS_LOC,
    BONE_INDICES_LOC,
    NORM_LOC,
    TANGENT_LOC,
    // BITANGENT_LOC,
    COLOR_LOC,
    CENTER_LOC,
};

void Shader_CompileShader(Shader *shader, const char *vPath, const char *fPath);
void Shader_DestroyShader(Shader *shader);

#endif