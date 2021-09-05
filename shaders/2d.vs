#version 120

attribute vec2 pos;
attribute vec2 uv;

uniform mat4 projView;

varying vec2 TexCoord;

void main(){

	TexCoord = uv;

	gl_Position = projView * vec4(pos, -1, 1);
}