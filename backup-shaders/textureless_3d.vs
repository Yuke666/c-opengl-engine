#version 120

attribute vec3 pos;
attribute vec2 uv;

uniform mat4 projView;
uniform mat4 view;
uniform mat4 model;

varying vec4 ScreenCoord;
varying vec2 TexCoord;

void main(){
	
	TexCoord = uv;

	ScreenCoord = projView * model * vec4(pos, 1);
	
	gl_Position = ScreenCoord;

}