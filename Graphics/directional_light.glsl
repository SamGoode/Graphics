#version 410

in vec2 vTexCoord;

out vec3 diffuseLight;
out vec3 specularLight;

uniform vec3 LightDirection;
uniform vec3 LightColor;

uniform sampler2D albedoSpecPass;
uniform sampler2D positionPass;
uniform sampler2D normalPass;

void main() {
	float S = texture(albedoSpecPass, vTexCoord).a;

	vec3 N = normalize( texture(normalPass, vTexCoord).xyz );
	vec3 L = normalize(LightDirection);

	float lambertTerm = max(0, dot(N, -L));

	vec3 position = texture(positionPass, vTexCoord).xyz;
	vec3 V = normalize(-position);
	
	vec3 H = normalize(V - L);

	float specTerm = pow(max(0, dot(H, N)), S);

	diffuseLight = lambertTerm * LightColor;
	specularLight = specTerm * LightColor;
}
