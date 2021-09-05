#version 120

attribute vec2 	pos;

varying vec2 	TexCoord;

void main(){
	
	gl_Position = vec4(pos, -1, 1);

	TexCoord = (pos + 1) * 0.5;
}