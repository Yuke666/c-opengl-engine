#version 120

varying vec2 		TexCoord;

uniform sampler2D 	tex;
uniform float 		texelSize;
uniform int 		blurSize = 4;

void main(){

	float blur = 0;

	float offset = (-float(blurSize)/2) + 0.5;

	for(int y = 0; y < blurSize; y++){

		blur += texture2D(tex, vec2(TexCoord.x, TexCoord.y + (offset * texelSize))).r;

		++offset;
	}

	// gl_FragColor = vec4(vec3(1) * (blur / float(blurSize)),1);
	gl_FragColor = vec4(1) * (blur / float(blurSize));
}