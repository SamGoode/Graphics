#version 410

in vec2 vTexCoord;

uniform mat4 ViewInverse;

uniform vec3 LightDirection; // view space
uniform vec3 LightColor;
uniform mat4 LightProjectionView;

uniform sampler2D albedoSpecPass;
uniform sampler2D positionPass;
uniform sampler2D normalPass;
uniform sampler2D shadowPass;

layout(location = 0) out vec3 diffuseLight;
layout(location = 1) out vec3 specularLight;


void main() {
	vec3 N = normalize( texture(normalPass, vTexCoord).xyz );
	vec3 L = normalize(LightDirection);

	float lambertTerm = max(0, dot(N, -L));

	vec3 position = texture(positionPass, vTexCoord).xyz;
	vec3 V = normalize(-position);
	
	vec3 H = normalize(V - L);

	float S = texture(albedoSpecPass, vTexCoord).a;
	float specTerm = pow(max(0, dot(H, N)), S * 32.f) * S;

	vec4 lightSpacePosition = LightProjectionView * ViewInverse * vec4(position, 1.f);
	vec3 projCoords = lightSpacePosition.xyz / lightSpacePosition.w;
	projCoords = projCoords * 0.5f + 0.5f;

	float closestDepth = texture(shadowPass, projCoords.xy).r;
	float currentDepth = projCoords.z;
	float shadow = currentDepth - 0.0005f < closestDepth ? 0.f : 1.f;
	
	diffuseLight = lambertTerm * LightColor * (1 - shadow);
	specularLight = specTerm * LightColor * (1 - shadow);
}
