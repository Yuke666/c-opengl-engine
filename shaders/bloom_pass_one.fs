#version 120

varying vec2 		TexCoord;

uniform sampler2D 	tex;
uniform float 		texelSize;
uniform float 		weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);


void main(){

	vec3 blur = texture2D(tex, TexCoord).rgb * weights[0];

	for(int x = 1; x < 5; x++){

		blur += texture2D(tex, TexCoord + vec2(texelSize * x, 0)).rgb * weights[x];
		blur += texture2D(tex, TexCoord - vec2(texelSize * x, 0)).rgb * weights[x];
	}

	gl_FragColor = vec4(blur,1);
}