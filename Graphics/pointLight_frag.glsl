#version 410

in vec3 vLightPosition;
in float vLightRadius;
in vec3 vLightColor;

uniform vec3 CameraPos;

uniform sampler2D albedoSpecPass;
uniform sampler2D positionPass;
uniform sampler2D normalPass;

layout(location = 0) out vec3 diffuseLight;
layout(location = 1) out vec3 specularLight;


void main() {
	vec2 texCoord = gl_FragCoord.xy / textureSize(positionPass, 0).xy;

	vec3 sPosition = texture(positionPass, texCoord).xyz;
	vec3 sNormal = normalize(texture(normalPass, texCoord).xyz);

	vec3 toView = normalize(CameraPos - sPosition);
	vec3 toLight = vLightPosition - sPosition;
	float dist = length(toLight);
	toLight = toLight / dist;

	float lambertTerm = max(0, dot(sNormal, toLight));
	float linearDrop = 1 - min(1, dist/vLightRadius);
	
	vec3 H = normalize(toView + toLight);

	float S = texture(albedoSpecPass, texCoord).a;
	float specTerm = pow(max(0, dot(H, sNormal)), S * 32.f) * S;

	diffuseLight = vLightColor * lambertTerm * linearDrop;
	specularLight = vLightColor * specTerm * linearDrop;
}