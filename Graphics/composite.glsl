#version 410

in vec2 vTexCoord;

out vec4 FragColor;

uniform vec3 AmbientLighting;

uniform sampler2D albedoTexture;
uniform sampler2D lightTexture;

void main() {
	vec3 albedo = texture(albedoTexture, vTexCoord).rgb;
	vec3 diffuseSpec = texture(lightTexture, vTexCoord).rgb * 2.f;

	vec3 ambient = albedo * AmbientLighting;

	FragColor = vec4(ambient + diffuseSpec, 1);
}