#version 120

varying vec2 		TexCoord;
uniform sampler2D 	tex;
uniform sampler2D 	ssao;
uniform sampler2D 	light;

void main(){

	vec3 shadows = texture2D(light, TexCoord).rgb;
	float ssaoVal = texture2D(ssao, TexCoord).a;
	vec3 lighting = texture2D(tex, TexCoord).rgb + 0.2;

	// gl_FragColor = vec4(shadows * ssaoVal, 1);
	gl_FragColor = vec4(shadows * lighting * ssaoVal, 1);
	// gl_FragColor = vec4(vec3((length(shadows)) / 4) * ssaoVal, 1);

	// gl_FragColor = texture2D(tex, TexCoord);
	// gl_FragColor = vec4(vec3(1,1,1) * texture2D(temp, TexCoord).b,1);
}