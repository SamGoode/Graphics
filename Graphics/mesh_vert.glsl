#version 410

in vec4 Position;
in vec4 Normal;

out vec4 vPosition;
out vec3 vNormal;

uniform mat4 ProjectionViewModel;
uniform mat4 ModelTransform;

void main() {
	vPosition = ModelTransform * Position;
	vNormal = (ModelTransform * Normal).xyz;
	gl_Position = ProjectionViewModel * Position;
}