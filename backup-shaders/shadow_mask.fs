#version 120

#define MAX_SOURCES 4

varying vec2 TexCoord;

uniform sampler2D shadowsTexture;
uniform sampler2D depthTexture;

uniform mat4 invProjView;

uniform vec4 darknesses;

uniform mat4 lightMatrices[MAX_SOURCES];

uniform int nSources;

vec2 poissonDisk[16] = vec2[](
    vec2( -0.94201624, -0.39906216 ),
    vec2( 0.94558609, -0.76890725 ),
    vec2( -0.094184101, -0.92938870 ),
    vec2( 0.34495938, 0.29387760 ),
    vec2( -0.91588581, 0.45771432 ),
    vec2( -0.81544232, -0.87912464 ),
    vec2( -0.38277543, 0.27676845 ),
    vec2( 0.97484398, 0.75648379 ),
    vec2( 0.44323325, -0.97511554 ),
    vec2( 0.53742981, -0.47373420 ),
    vec2( -0.26496911, -0.41893023 ),
    vec2( 0.79197514, 0.19090188 ),
    vec2( -0.24188840, 0.99706507 ),
    vec2( -0.81409955, 0.91437590 ),
    vec2( 0.19984126, 0.78641367 ),
    vec2( 0.14383161, -0.14100790 )
);

void main(){

	vec4 pos = invProjView * vec4((TexCoord * 2) - 1, (texture2D(depthTexture, TexCoord).r * 2) - 1, 1);

	vec4 curr;

	float occluded = 0;

	for(int k = 0; k < nSources; k++){

		curr = lightMatrices[k] * pos;

		curr /= curr.w;

		curr.xyz += 1;
		curr.xyz *= 0.5;

		if(curr.x < 0 || curr.x > 1 || curr.y < 0 || curr.y > 1 || curr.z < 0 || curr.z > 1)
			continue;

		for(int j = 0; j < 16; j++){

			vec3 sample = vec3(curr.xy + (poissonDisk[j]/200.0), curr.z - 0.0005);

			sample /= curr.w;
	
			if(sample.x < 0 || sample.x > 1 || sample.y < 0 || sample.y > 1 || sample.z < 0 || sample.z > 1)
				continue;

			sample.x /= MAX_SOURCES;
			sample.x += (1.0/MAX_SOURCES) * k;

			float depth = texture2D(shadowsTexture, sample.xy).r;

			if(depth < sample.z)
				occluded += darknesses[k] / 16.0;
		}
	}

	gl_FragColor = vec4(1-occluded,1,1,1);
}