#version 120

#define KERNEL_SIZE 32

varying vec2 		TexCoord;

uniform float 		radius = 1.0;
uniform sampler2D 	depthTexture;
uniform sampler2D 	normalTexture;
uniform sampler2D 	randomTexture;
uniform vec2		noiseScale;
uniform mat4 		proj;

varying vec2 		viewRay;
uniform vec2 		texelSize;

uniform vec3 		kernel[KERNEL_SIZE];

float DepthToViewSpace(float screenDepth){

	return proj[2][3] / ((2 * screenDepth - 1) - proj[2][2]);
}

vec3 sampleOffset(sampler2D tex, vec2 offset){

	return texture2D(tex, TexCoord.xy + ((offset - 0.5) * texelSize)).rgb;
}

void main(){

	vec4 top;
	vec4 bottom;
	vec4 left;
	vec4 right;
	vec4 topLeft;
	vec4 topRight;
	vec4 bottomLeft;
	vec4 bottomRight;

	top.rgb         = sampleOffset(normalTexture, vec2(0, -1)).xyz;
	bottom.rgb      = sampleOffset(normalTexture, vec2(0, 1)).xyz;
	left.rgb        = sampleOffset(normalTexture, vec2(-1, 0)).xyz;
	right.rgb       = sampleOffset(normalTexture, vec2(1, 0)).xyz;
	topLeft.rgb     = sampleOffset(normalTexture, vec2(-1, -1)).xyz;
	topRight.rgb    = sampleOffset(normalTexture, vec2(1, -1)).xyz;
	bottomLeft.rgb  = sampleOffset(normalTexture, vec2(-1, 1)).xyz;
	bottomRight.rgb = sampleOffset(normalTexture, vec2(1, 1)).xyz;
	top.a         = sampleOffset(depthTexture, vec2(0, -1)).r;
	bottom.a      = sampleOffset(depthTexture, vec2(0, 1)).r;
	left.a        = sampleOffset(depthTexture, vec2(-1, 0)).r;
	right.a       = sampleOffset(depthTexture, vec2(1, 0)).r;
	topLeft.a     = sampleOffset(depthTexture, vec2(-1, -1)).r;
	topRight.a    = sampleOffset(depthTexture, vec2(1, -1)).r;
	bottomLeft.a  = sampleOffset(depthTexture, vec2(-1, 1)).r;
	bottomRight.a = sampleOffset(depthTexture, vec2(1, 1)).r;
	
	vec4 sx = -topLeft - 2 * left - bottomLeft + topRight   + 2 * right  + bottomRight;
	vec4 sy = -topLeft - 2 * top  - topRight   + bottomLeft + 2 * bottom + bottomRight;
	vec4 sobel = sqrt(sx * sx + sy * sy);

	float lightness = length(sobel) > 0.5 ? 1 : 0;

	gl_FragColor = vec4(vec3(lightness), 1);

	// vec3 pos;

	// pos.z = DepthToViewSpace(texture2D(depthTexture, TexCoord).r);

	// pos.xy = viewRay * pos.z;

	// vec3 normal = texture2D(normalTexture, TexCoord).xyz;

	// if(length(normal.xyz) == 0) discard;

	// // normal = normalize(normal);

	// vec3 random;
	// random.xy = texture2D(randomTexture, TexCoord * noiseScale).xy * 2.0 - 1;
	// random.z = 0;
	// // random = normalize(random);

	// vec3 tangent = normalize(random - normal * dot(random, normal));
	// vec3 bitangent = cross(normal, tangent);
	// mat3 tbn = mat3(tangent, bitangent, normal);

	// float occlusion = 0;

	// for(int k = 0; k < KERNEL_SIZE; k++){

	// 	vec3 sample = tbn * kernel[k];

	// 	sample = (sample * radius) + pos;

	// 	vec4 offset = vec4(sample, 1.0);

	// 	offset = proj * offset;

	// 	offset.xy = ((offset.xy/offset.w) * 0.5) + 0.5;

	// 	float depth = DepthToViewSpace(texture2D(depthTexture, offset.xy).r);

	// 	// float rangeCheck = abs(pos.z - depth) < radius ? 1 : 0;
	// 	// occlusion += rangeCheck * (depth >= sample.z ? 1.0 : 0.0);

	// 	// float rangeCheck = abs(pos.z - depth) < radius ? 1 : 0;
	// 	// occlusion += rangeCheck * (depth <= sample.z ? 0.0 : 1.0);
		
	// 	float rangeCheck = smoothstep(0.0, 1.0, radius / abs(pos.z - depth));
	// 	occlusion += (depth >= sample.z ? 0.0 : 1.0) * rangeCheck;
	// }

	// occlusion = 1 - (occlusion / KERNEL_SIZE);

	// gl_FragColor = vec4(vec3(occlusion),1);
}