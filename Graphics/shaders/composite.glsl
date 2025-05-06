#version 410

in vec2 vTexCoord;

uniform vec3 AmbientLighting;

uniform sampler2D albedoSpecPass;
uniform sampler2D diffuseLightPass;
uniform sampler2D specularLightPass;
uniform sampler2D shadowPass;

out vec4 FragColor;


void main() {
	vec4 albedoSpec = texture(albedoSpecPass, vTexCoord);
	vec3 diffuseLight = texture(diffuseLightPass, vTexCoord).rgb;
	vec3 specularLight = texture(specularLightPass, vTexCoord).rgb;

	float depth = texture(shadowPass, vTexCoord).r;

	vec3 ambient = albedoSpec.rgb * AmbientLighting;
	vec3 diffuse = albedoSpec.rgb * diffuseLight;
	vec3 specular = specularLight;

	FragColor = vec4(ambient, 1);
	//FragColor = vec4(ambient + diffuse + specular, 1);
	//FragColor = vec4(vec3(depth), 1.f);
}