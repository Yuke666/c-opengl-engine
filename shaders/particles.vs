#version 120

attribute vec3 center;
attribute vec2 uv;
attribute vec2 pos;

uniform mat4 projView;
uniform mat4 model;

// uniform vec3 camPos;
uniform vec3 camRight;
uniform vec3 camUp;

varying vec2 TexCoord;
varying vec4 Position;

void main(){

	vec4 modelCenter = model * vec4(center, 1);

	TexCoord = uv;

	// vec3 viewDir = normalize(camPos - modelCenter.xyz);

 //    vec3 cameraUp = normalize(cross(viewDir, camRight));
	// vec3 cameraRight = normalize(cross(cameraUp, viewDir));

	// vec3 right = vec3(cameraRight * pos.x);
	// vec3 up = vec3(cameraUp * pos.y);

	vec3 right = vec3(camRight * pos.x);
	vec3 up = vec3(camUp * pos.y);

	vec4 newpos = modelCenter;
	newpos.x += right.x + up.x;
	newpos.y += right.y + up.y;
	newpos.z += right.z + up.z;

	Position = projView * newpos;

	gl_Position = Position;
}