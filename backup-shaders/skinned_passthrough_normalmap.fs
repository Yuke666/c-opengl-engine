#version 120

varying vec2 TexCoord;
varying mat3 TBN;

uniform sampler2D tex;

uniform float specularHardness;
uniform float normalFactor = 1;

void main(){

	vec3 norm = normalize(TBN * normalize(texture2D(tex, TexCoord).rgb * 2 - 1));

	gl_FragData[0] = vec4(norm.xyz, specularHardness);
}