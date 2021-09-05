#version 120

attribute vec2 	pos;

varying vec2 	TexCoord;

uniform float 	aspect;
uniform float 	tanHalfFov;

varying vec2 	ViewRay;

void main(){
	
	gl_Position = vec4(pos, 1, 1);

	TexCoord = (pos * 0.5) + 0.5;

	ViewRay.x = pos.x * aspect * tanHalfFov;
	ViewRay.y = pos.y * tanHalfFov;
}