#version 120

attribute vec2 	pos;

varying vec4 	TexCoord;
uniform vec2 	texelSize;
uniform float 	FXAA_SUBPIX_SHIFT = 1.0/4.0;

void main(){
	
	gl_Position = vec4(pos, -1, 1);

	TexCoord.xy = (pos + 1) * 0.5;
	TexCoord.zw = TexCoord.xy - (texelSize * (0.5 + FXAA_SUBPIX_SHIFT));
}