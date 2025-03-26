#version 410

in vec2 vTexCoord;

//out vec3 diffuseLight;
//out vec3 specularLight;
out vec3 outColor;

uniform vec3 CameraPos;

uniform vec3 LightDirection;
uniform vec3 LightColor;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

uniform sampler2D positionTexture;
uniform sampler2D normalTexture;

void main() {
	vec3 Kd = texture(diffuseTexture, vTexCoord).xyz;
	vec4 spec = texture(specularTexture, vTexCoord);
	vec3 Ks = spec.rgb;
	float S = spec.a;

	vec3 N = normalize( texture(normalTexture, vTexCoord).xyz );
	vec3 position = texture(positionTexture, vTexCoord).xyz;

	float lambertTerm = max(0, dot(N, -LightDirection));

	vec3 V = normalize(CameraPos - position);
	vec3 R = reflect(LightDirection, N);

	float specTerm = pow(max(0, dot(R, V)), S);

	vec3 diffuse = Kd * lambertTerm * LightColor;
	vec3 specular = Ks * specTerm * LightColor;

	outColor = vec3(diffuse + specular);
}

//Ka * AmbientLighting + Kd * lambert * LightColor + ks * specularTerm * LightColor

// AmbientLighting + albedo * lambert * LightColor + ks * specularTerm * LightColor