#version 430 core

#include "common.h"


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


layout(binding = PROJECTIONVIEW_UBO, std140) uniform PVMatrices {
	mat4 View;
	mat4 Projection;
	mat4 ViewInverse;
	mat4 ProjectionInverse;
};

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