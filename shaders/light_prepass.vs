#version 120

attribute vec3 	pos;

uniform mat4 	projView;
uniform vec3 	center;
uniform float 	radius;

varying vec4 	Position;

void main(){

	if(radius == 0){

		Position = vec4(pos.xy, -1, 1);
	
	} else {

		Position = projView * vec4((pos * radius) + center, 1);
	}

	gl_Position = Position;
}