#version 120

varying vec2 TexCoord;

uniform sampler2D tex;
uniform vec4 uniformColor;

void main(){
	
	gl_FragColor = texture2D(tex, TexCoord) * uniformColor;
}