#version 410

in vec4 vPosition;
in vec3 vNormal;

out vec4 FragColor;

uniform vec3 cameraPos;

uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform float specExp;

uniform vec3 LightDirection;

void main() {
	vec3 N = normalize(vNormal);
	vec3 L = normalize(LightDirection);

	float lambert = max(0, min(1, dot(N, -L)));

	vec3 V = normalize(cameraPos - vPosition.xyz);
	vec3 R = reflect(L, N);

	float specularTerm = pow(max(0, dot(R, V)), specExp);

	vec3 ambient = Ka;
	vec3 diffuse = Kd * lambert;
	vec3 specular = Ks * specularTerm;

	FragColor = vec4(ambient + diffuse + specular, 1);
}