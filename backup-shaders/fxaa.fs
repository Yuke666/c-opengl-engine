#version 120

#define FXAA_REDUCE_MIN (1.0/128.0)
#define FXAA_REDUCE_MUL (1.0/8.0)
#define FXAA_SPAN_MAX 8.0

varying vec4 TexCoord;
uniform sampler2D tex;
uniform sampler2D depthTexture;
uniform vec2 texelSize;

vec3 sampleOffset(vec2 offset){

	return texture2D(tex, TexCoord.zw + (offset * texelSize)).rgb;
}

vec3 fxaa(){

   vec3 rgbNW = texture2D(tex, TexCoord.zw).rgb;
   vec3 rgbNE = sampleOffset(vec2(1,0)).rgb;
   vec3 rgbSW = sampleOffset(vec2(0,1)).rgb;
   vec3 rgbSE = sampleOffset(vec2(1,1)).rgb;
   vec3 rgbM  = texture2D(tex, TexCoord.xy).rgb;

    vec3 luma = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);

   float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
   float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE))); 
   
   vec2 dir;
   dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
   dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
   
   float dirReduce = max(
        (lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL),
        FXAA_REDUCE_MIN);

    float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = min(vec2( FXAA_SPAN_MAX,  FXAA_SPAN_MAX), 
          max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), 
          dir * rcpDirMin)) * texelSize.xy;

    vec3 rgbA = (1.0/2.0) * (
        texture2D(tex, TexCoord.xy + dir * (1.0/3.0 - 0.5)).rgb +
        texture2D(tex, TexCoord.xy + dir * (2.0/3.0 - 0.5)).rgb);
    
    vec3 rgbB = rgbA * (1.0/2.0) + (1.0/4.0) * (
        texture2D(tex, TexCoord.xy + dir * (0.0/3.0 - 0.5)).rgb +
        texture2D(tex, TexCoord.xy + dir * (3.0/3.0 - 0.5)).rgb);
    
    float lumaB = dot(rgbB, luma);

    if((lumaB < lumaMin) || (lumaB > lumaMax)) return rgbA;

    return rgbB;
}

void main(){

	gl_FragColor = vec4(fxaa(), 1);
}