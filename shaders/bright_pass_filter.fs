#version 120

varying vec2 		TexCoord;

uniform sampler2D 	tex;

uniform float threshold = 1.0;

void main(){

	vec3 color = texture2D(tex, TexCoord).rgb;

	if(dot(color, vec3(0.2126, 0.7152, 0.0722)) > threshold){

		gl_FragColor = vec4(color,1);
	
	} else {
	
		gl_FragColor = vec4(0,0,0,1);
	}

}