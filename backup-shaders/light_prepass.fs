#version 120

varying vec4 Position;

uniform sampler2D depthTexture;
uniform sampler2D normalTexture;

uniform mat4 invProj;
uniform vec3 lightCenter;
uniform vec3 color = vec3(1,1,1);
uniform vec3 ambient = vec3(0.3);
uniform float lightAmbient;
uniform float lightLinear;
uniform float lightQuadratic;
uniform float lightConstant;

void main(){

	vec4 pos = Position / Position.w;

	vec2 texCoord = (pos.xy + 1) * 0.5;

	vec4 normalSample = texture2D(normalTexture, texCoord);

	float shininess = normalSample.a;

	vec3 normal = normalSample.xyz;

	if(length(normal) == 0)
		discard;

	pos = invProj * vec4(pos.xy, (texture2D(depthTexture, texCoord).r * 2) - 1, 1);

	pos.xyz /= pos.w;

	vec3 diffuse = ambient + (lightAmbient * color);
	vec3 specular = vec3(0,0,0);

    vec3 lightDir = lightCenter - pos.xyz;

	float distance = length(lightDir);

	lightDir = normalize(lightDir);

	float lambertian = dot(lightDir, normal);

	if(lambertian > 0){

		vec3 viewDirection = normalize(-pos.xyz);

		vec3 halfDir = normalize(lightDir + viewDirection);
		
		float spec = pow(max(dot(normal, halfDir), 0.0), shininess);
    	
    	// float spec = pow(max(0, dot(reflect(lightDir, normal), viewDirection)), 3));

		float attenuation = 1.0 / (1.0 + lightLinear * distance + lightQuadratic * distance * distance);

		specular = color * spec * attenuation;
	    diffuse += color * lambertian * attenuation;

	}

    gl_FragData[0] = vec4(diffuse, 1);
    // gl_FragData[0] = vec4(normal * 0.5 + 0.5, 1);
    gl_FragData[1] = vec4(specular,1);
}