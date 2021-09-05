#version 120

varying vec4 ReflectionPosition;
varying vec2 TexCoord;

uniform sampler2D tex;
uniform sampler2D floorTex;

void main(){
	
	gl_FragColor = vec4(0,1,0,1);
    // vec2 rCoord = ((ReflectionPosition.xy / ReflectionPosition.w) * 0.5) + 0.5;

    // gl_FragColor = texture2D(floorTex, TexCoord) + (texture2D(tex, rCoord) * 0.3);
    // gl_FragColor = texture2D(tex, rCoord);
}