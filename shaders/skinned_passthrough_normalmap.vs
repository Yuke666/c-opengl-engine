#version 120

attribute vec3 pos;
attribute vec2 uv;
attribute vec3 norm;
attribute vec4 tangent;
attribute vec4 weights;
attribute vec4 boneIndices;

varying vec2 TexCoord;
varying mat3 TBN;

uniform mat4 projView;
uniform mat4 view;
uniform mat4 model;
uniform mat3 modelInvTranspose;

uniform vec4 bones[64 * 3];

void main(){

	vec4 r1;
	vec4 r2;
	vec4 r3;

	if(boneIndices.x >= 0){
		r1 = bones[(int(boneIndices.x)*3)] * weights.x;
		r2 = bones[(int(boneIndices.x)*3)+1] * weights.x;
		r3 = bones[(int(boneIndices.x)*3)+2] * weights.x;
	} else {
		r1 = vec4(1,0,0,0);
		r2 = vec4(0,1,0,0);
		r3 = vec4(0,0,1,0);
	}

	if(boneIndices.y >= 0){
		r1 += bones[(int(boneIndices.y)*3)] * weights.y;
		r2 += bones[(int(boneIndices.y)*3)+1] * weights.y;
		r3 += bones[(int(boneIndices.y)*3)+2] * weights.y;
	}

	if(boneIndices.z >= 0){
		r1 += bones[(int(boneIndices.z)*3)] * weights.z;
		r2 += bones[(int(boneIndices.z)*3)+1] * weights.z;
		r3 += bones[(int(boneIndices.z)*3)+2] * weights.z;
	}

	if(boneIndices.w >= 0){
		r1 += bones[(int(boneIndices.w)*3)] * weights.w;
		r2 += bones[(int(boneIndices.w)*3)+1] * weights.w;
		r3 += bones[(int(boneIndices.w)*3)+2] * weights.w;
	}

	vec4 pos4 = vec4(pos, 1);

	vec4 position = pos4;

	position.x = dot(r1, pos4);
	position.y = dot(r2, pos4);
	position.z = dot(r3, pos4);
	position.w = 1;

	gl_Position = projView * model * position;

	TexCoord = uv;

	// mat3 matrix = transpose(inverse(mat3(r1.xyz, r2.xyz, r3.xyz)));
	mat3 matrix = mat3(r1.xyz, r2.xyz, r3.xyz);

	TBN[2] = normalize(matrix * modelInvTranspose * norm);
	TBN[0] = normalize(matrix * modelInvTranspose * tangent.xyz);

	TBN[0] = normalize(TBN[0] - TBN[2] * dot(TBN[0], TBN[2]));

	TBN[1] = normalize(cross(TBN[2], TBN[0])) * tangent.w;

	TBN = mat3(view) * TBN;
}