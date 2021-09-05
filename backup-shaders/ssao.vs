#version 120

attribute vec2 	pos;

varying vec2 	TexCoord;

uniform float 	aspect;
uniform float 	tanHalfFov;

varying vec2 	viewRay;

void main(){
	
	gl_Position = vec4(pos, -1, 1);

	TexCoord = (pos + 1) * 0.5;

	viewRay.x = -pos.x * aspect * tanHalfFov;
	viewRay.y = -pos.y * tanHalfFov;
}