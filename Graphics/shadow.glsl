#version 410

layout(location = 0) in vec4 Position;
layout(location = 1) in vec4 Normal;
layout(location = 2) in vec2 TexCoord;

// per-instance data
layout(location = 3) in struct instanceData {
	mat4 transform;
	vec3 baseColor;
	float gloss;
} instance;

uniform mat4 LightProjectionView;

void main() {
	gl_Position = LightProjectionView * instance.transform * Position;
}