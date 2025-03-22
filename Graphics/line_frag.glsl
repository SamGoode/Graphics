#version 410

out vec4 FragColor;

uniform vec3 BaseColor;

void main() {
	FragColor = vec4(BaseColor, 1);
}