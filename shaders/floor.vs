#version 120

attribute vec3 pos;
attribute vec2 uv;

uniform mat4 projView;
uniform mat4 reflectionMatrix;
uniform mat4 model;

varying vec4 ReflectionPosition;
varying vec2 TexCoord;

void main(){

	TexCoord = uv;
	
	vec4 modelPos = model * vec4(pos, 1);

	gl_Position = projView * modelPos;

	ReflectionPosition = reflectionMatrix * modelPos; 
}