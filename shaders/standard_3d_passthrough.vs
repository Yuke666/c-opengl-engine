#version 120

attribute vec3 pos;
attribute vec3 norm;

varying vec3 Normal;

uniform mat4 projView;
uniform mat4 model;
uniform mat4 view;
uniform mat3 modelInvTranspose;

void main(){
	
	gl_Position = projView * model * vec4(pos, 1);

	Normal = mat3(view) * normalize(modelInvTranspose * norm);
}