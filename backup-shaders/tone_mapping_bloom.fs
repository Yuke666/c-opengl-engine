#version 120

varying vec2 		TexCoord;

uniform sampler2D 	tex;
uniform sampler2D 	bloomTexture;

void main(){

	vec3 color = texture2D(tex, TexCoord).rgb + texture2D(bloomTexture, TexCoord).rgb;
	
	// color *= 1;

	// color = max(vec3(0),color-0.004);
	// color = (color*(6.2*color+.5))/(color*(6.2*color+1.7)+0.06);
	
	gl_FragColor = vec4(color,1);
}