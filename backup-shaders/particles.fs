#version 120

varying vec2 TexCoord;
varying vec4 Position;

uniform mat4 proj;

uniform float softness = 10;

uniform sampler2D tex;
uniform sampler2D depthTexture;

float DepthToViewSpace(float screenDepth){

	return proj[3][2] / (2 * screenDepth - 1 - proj[2][2]);
}

void main(){

	float depth = texture2D(depthTexture, ((Position.xy / Position.w) * 0.5) + 0.5).r;


	float fragDepth = DepthToViewSpace((Position.z / Position.w) * 0.5 + 0.5);

	float opacity = 1;


	if(depth != 1){

		depth = DepthToViewSpace(depth);
		opacity = (depth - fragDepth) * softness;
	}


	vec4 color = texture2D(tex, TexCoord);

	color *= clamp(opacity, 0, 1);

	gl_FragColor = color;
}

