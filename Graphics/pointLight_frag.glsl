#version 410

in vec3 vLightPosition;
in float vLightRadius;
in vec3 vLightColor;

uniform sampler2D albedoSpecPass;
uniform sampler2D positionPass;
uniform sampler2D normalPass;

layout(location = 0) out vec3 diffuseLight;
layout(location = 1) out vec3 specularLight;


void main() {
	vec2 texCoord = gl_FragCoord.xy / textureSize(positionPass, 0).xy;

	vec3 sPosition = texture(positionPass, texCoord).xyz;
	vec3 sNormal = texture(normalPass, texCoord).xyz;

	vec3 toLight = vLightPosition - sPosition;
	vec3 toView = normalize(-sPosition);

	float lambertTerm = max(0, dot(sNormal, toLight));
	float linearDrop = 1 - min(1, length(toLight)/vLightRadius);
	
	vec3 H = normalize(toView + normalize(toLight));
	float S = texture(albedoSpecPass, texCoord).a;
	float specTerm = pow(max(0, dot(H, sNormal)), S);

	diffuseLight = vLightColor * lambertTerm * linearDrop;
	specularLight = vLightColor * specTerm * linearDrop;
}