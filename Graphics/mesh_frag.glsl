#version 410

in vec4 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;

in vec3 Ka;
in vec3 Kd;
in vec3 Ks;
in float S;

out vec4 FragColor;

uniform vec3 CameraPos;

uniform vec3 AmbientLighting;
uniform vec3 LightColor;
uniform vec3 LightDirection;

uniform sampler2D diffuseTex;

void main() {
	vec3 N = normalize(vNormal);
	vec3 L = normalize(LightDirection);

	float lambert = max(0, min(1, dot(N, -L)));

	vec3 V = normalize(CameraPos - vPosition.xyz);
	vec3 R = reflect(L, N);

	float specularTerm = pow(max(0, dot(R, V)), S);

	vec3 textureColor = texture(diffuseTex, vTexCoord).rgb;

	vec3 ambient = Ka * textureColor * AmbientLighting;
	vec3 diffuse = Kd * textureColor * lambert * LightColor;
	vec3 specular = Ks * specularTerm * LightColor;

	FragColor = vec4(ambient + diffuse + specular, 1);
}