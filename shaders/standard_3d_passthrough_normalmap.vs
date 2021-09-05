#version 120

attribute vec3 pos;
attribute vec2 uv;
attribute vec3 norm;
attribute vec4 tangent;

varying vec2 TexCoord;
varying mat3 TBN;

uniform mat4 projView;
uniform mat4 model;
uniform mat4 view;
uniform mat3 modelViewInvTranspose;

void main(){
	
	gl_Position = projView * model * vec4(pos, 1);

	TBN[2] = modelViewInvTranspose * norm;
	TBN[0] = modelViewInvTranspose * tangent.xyz;

	TBN[1] = normalize(cross(TBN[2], TBN[0])) * tangent.w;

	TBN = mat3(view) * TBN;

	TexCoord = uv;
}