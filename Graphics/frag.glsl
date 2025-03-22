#version 150

in vec4 vColour;
in vec3 vNormal;

out vec4 FragColor;

uniform vec3 LightDirection;

void main() {
	vec3 N = vNormal;
	vec3 L = normalize(LightDirection);

	float lambert = max(0, min(1, dot(N, -L)));

	FragColor = vec4(vColour.xyz + vec3(0.4f) * lambert, 1);
}