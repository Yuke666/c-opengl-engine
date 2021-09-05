#version 120

varying vec3 Normal;

uniform mat4 proj;
uniform float invProjWW;
uniform float specularHardness;

void main(){

	gl_FragData[0] = vec4(normalize(Normal), specularHardness);
	
	float z = -1.0f / ((depth / proj[3][2]) + invProjWW);
	
	gl_FragData[1] = vec4(z,1,1,1);

}