#version 150

in vec4 vColor;
in vec3 vNormal;

out vec4 FragColor;

uniform vec3 LightDirection;
uniform vec3 BaseColor;

void main() {
	vec3 N = normalize(vNormal);
	vec3 L = normalize(LightDirection);

	float lambert = max(0, min(1, dot(N, -L)));

	FragColor = vec4(BaseColor + vec3(lambert * 0.4f), 1);//vec4(vColor.xyz + vec3(0.4f) * lambert, 1);
}