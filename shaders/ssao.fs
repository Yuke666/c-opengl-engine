#version 120

#define KERNEL_SIZE 32

varying vec2 		TexCoord;
varying vec2 		ViewRay;

uniform float 		radius = 0.05;
uniform sampler2D 	depthTexture;
uniform sampler2D 	normalTexture;
uniform sampler2D 	randomTexture;
uniform vec2		noiseScale;
uniform mat4 		proj;
uniform float 		invProjWW;

uniform vec2 		texelSize;

uniform vec3 		kernel[KERNEL_SIZE];

float ScreenSpaceToViewSpaceZ(vec2 coord){

	float depth = texture2D(depthTexture, coord).r * 2 - 1;

	return -1.0f / ((depth / proj[3][2]) + invProjWW);
}

vec3 ScreenSpaceToViewSpace(vec2 coord){

	float depth = texture2D(depthTexture, coord).r * 2 - 1;

	float w = (depth / proj[3][2]) + invProjWW;

    coord = coord * 2 - 1;

	return vec3(coord.x / proj[0][0], coord.y / proj[1][1], -1.0f) / w;
}

vec3 sampleOffset(sampler2D tex, vec2 offset){

	return texture2D(tex, TexCoord.xy + ((offset - 0.5) * texelSize)).rgb;
}

float DepthToViewSpace(float screenDepth){

	return proj[3][2] / ((2 * screenDepth - 1) - proj[2][2]);
}


void main(){

	vec3 pos;

	pos = ScreenSpaceToViewSpace(TexCoord);

	vec3 normal = texture2D(normalTexture, TexCoord).xyz;

	if(length(normal) == 0)
		discard;

	vec3 random;
	random.xy = texture2D(randomTexture, TexCoord * noiseScale).xy * 2.0 - 1;
	random.z = 0;
	// random = normalize(random);

	vec3 tangent = normalize(random - normal * dot(random, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 tbn = mat3(tangent, bitangent, normal);

	float occlusion = 0;

	// for(int k = 0; k < KERNEL_SIZE; k++){

		// vec3 sample = tbn * kernel[k];

		vec3 sample = normal;

		sample = pos + (sample * radius);

		vec4 offset = vec4(sample, 1.0);

		offset = proj * offset;

		offset.xy = ((offset.xy/offset.w) * 0.5) + 0.5;

		float depth = ScreenSpaceToViewSpaceZ(offset.xy);

		// float rangeCheck = abs(pos.z - depth) < radius ? 1 : 0;
		// occlusion += rangeCheck * (depth >= sample.z ? 1.0 : 0.0);

		// occlusion += depth  10;

		occlusion += abs(pos.z - depth) > radius ? 1 : 0;

		// float rangeCheck = abs(pos.z - depth) < radius ? 1 : 0;
		// occlusion += rangeCheck * (depth <= sample.z ? 0.0 : 1.0);
		
		// float rangeCheck = smoothstep(0.0, 1.0, radius / abs(pos.z - depth));
		// occlusion += (depth >= sample.z ? 0.0 : 1.0) * rangeCheck;
	// }

	// occlusion = 1 - (occlusion / KERNEL_SIZE);
	// occlusion = (occlusion / KERNEL_SIZE);

	gl_FragColor = vec4(vec3(dot(normalize(-ScreenSpaceToViewSpace(offset.xy)), normal)),1);
}