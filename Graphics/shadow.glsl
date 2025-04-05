#version 410

layout(location = 0) in vec3 Position;

// per-instance data
layout(location = 3) in struct instanceData {
	mat4 transform;
	vec3 baseColor;
	float gloss;
} instance;

uniform mat4 LightProjectionView;

void main() {
	gl_Position = LightProjectionView * instance.transform * vec4(Position, 1);
}