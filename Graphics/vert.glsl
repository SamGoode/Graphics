#version 150

in vec4 Position;
in vec4 Colour;
in vec4 Normal;

out vec4 vColour;
out vec3 vNormal;

uniform mat4 ProjectionView;

void main() {
	vColour = Colour;
	vNormal = normalize(Normal.xyz);
	gl_Position = ProjectionView * Position;
}