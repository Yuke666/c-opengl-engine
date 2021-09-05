#version 120

varying vec3 Normal;

uniform float specularHardness;

void main(){

	gl_FragData[0] = vec4(normalize(Normal), specularHardness);
}