#version 120

varying vec2 TexCoord;

uniform vec4 uniformColor;

void main(){
	
	gl_FragColor = uniformColor;
}