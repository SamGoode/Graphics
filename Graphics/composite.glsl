#version 410

in vec2 vTexCoord;

out vec4 FragColor;

uniform vec3 AmbientLighting;

uniform sampler2D albedoTexture;
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

void main() {
	vec4 albedo = texture(albedoTexture, vTexCoord).rgba;

	vec3 ambient = albedo.rgb * AmbientLighting;
	vec3 diffuse = texture(diffuseTexture, vTexCoord).rgb;
	vec3 specular = texture(specularTexture, vTexCoord).rgb;

	//FragColor = vec4(vec3(1), 0);
	FragColor = vec4(ambient + diffuse + specular, albedo.a);
}