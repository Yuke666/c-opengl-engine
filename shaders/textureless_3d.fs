#version 120

varying vec2 TexCoord;
varying vec4 ScreenCoord;

uniform sampler2D tex;
uniform sampler2D ssao;
uniform sampler2D shadowMask;
uniform sampler2D lightSpecular;
uniform sampler2D lightDiffuse;

uniform float ambient;
uniform vec4 diffuse;
uniform vec4 specular;
uniform float gamma = 2.2;

uniform int mode = 0;

void main(){

	vec2 coord = (ScreenCoord.xy/ScreenCoord.w) * 0.5 + 0.5;
	
	vec3 diffuseSample = texture2D(lightDiffuse, coord).rgb;
	vec3 specularSample = texture2D(lightSpecular, coord).rgb;

	float shadows = texture2D(shadowMask, coord).r;
	float ssaoVal = texture2D(ssao, coord).r;

	float shading = ssaoVal * shadows;

	vec3 color = shading * ((specularSample * specular.a * specular.rgb) + (diffuseSample * diffuse.rgb));
	// // vec3 color = shading * ((specularSample * specular.a) + (diffuse.rgb));

	if(mode == 0)
		gl_FragColor = vec4(color,1);
	else if(mode == 1)
		gl_FragColor = vec4(diffuseSample,1);
	else if(mode == 2)
		gl_FragColor = vec4(specularSample,1);
	else if(mode == 3)
		gl_FragColor = vec4(vec3(shading),1);
	else if(mode == 4)
		gl_FragColor = vec4(diffuse.rgb, 1);
	else
		gl_FragColor = vec4(diffuse.rgb, 1);

	// gl_FragColor = vec4(1-shading,1);
}