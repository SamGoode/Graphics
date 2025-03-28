#version 410

in vec2 vTexCoord;

out vec4 FragColor;

uniform vec3 AmbientLighting;

uniform sampler2D albedoSpecPass;
uniform sampler2D diffuseLightPass;
uniform sampler2D specularLightPass;

void main() {
	vec4 albedoSpec = texture(albedoSpecPass, vTexCoord);
	vec3 diffuseLight = texture(diffuseLightPass, vTexCoord).rgb;
	vec3 specularLight = texture(specularLightPass, vTexCoord).rgb;

	vec3 ambient = albedoSpec.rgb * AmbientLighting;
	vec3 diffuse = albedoSpec.rgb * diffuseLight;
	vec3 specular = specularLight * 0.5f;

	FragColor = vec4(ambient + diffuse + specular, 1);
}