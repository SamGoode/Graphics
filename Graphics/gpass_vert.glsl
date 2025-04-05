#version 410

// per-vertex data
layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TexCoord;

// per-instance data
layout(location = 3) in struct instanceData {
	mat4 transform;
	vec3 baseColor;
	float gloss;
} instance;


uniform mat4 View;
uniform mat4 Projection;

out vec4 vPosition;
out vec3 vNormal;
out vec2 vTexCoord;

out vec3 Ka;
out float S;


void main() {
	vPosition = View * instance.transform * vec4(Position, 1);
	vNormal = (View * instance.transform * vec4(Normal, 0)).xyz;
	vTexCoord = TexCoord;

	Ka = instance.baseColor;
	S = instance.gloss;

	gl_Position = Projection * vPosition;
}