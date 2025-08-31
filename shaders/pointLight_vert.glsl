#version 430 core

#include "common.h"


// per-instance data
layout(location = 0) in struct instanceData {
	vec3 lightPosition;
	float lightRadius;
	vec3 lightColor;
} instance;


layout(binding = PROJECTIONVIEW_UBO, std140) uniform PVMatrices {
	mat4 View;
	mat4 Projection;
	mat4 ViewInverse;
	mat4 ProjectionInverse;
};

out vec3 vLightPosition;
out float vLightRadius;
out vec3 vLightColor;


void main() {
	// Cube vertices
	const vec3 vertices[8] = vec3[8](
		vec3(1, 1, -1),
		vec3(1, -1, -1),
		vec3(-1, -1, -1),
		vec3(-1, 1, -1),
		vec3(1, 1, 1),
		vec3(1, -1, 1),
		vec3(-1, -1, 1),
		vec3(-1, 1, 1)
	);

	const uint indices[36] = uint[36](
		4, 6, 5, 4, 7, 6,
		3, 1, 2, 3, 0, 1,
		5, 0, 4, 5, 1, 0,
		3, 2, 6, 3, 6, 7,
		3, 4, 0, 3, 7, 4,
		2, 1, 5, 2, 5, 6
	);

	vec3 Position = vertices[indices[gl_VertexID]];

	vLightPosition = (View * vec4(instance.lightPosition, 1)).xyz;
	vLightRadius = instance.lightRadius;
	vLightColor = instance.lightColor;
	
	//gl_Position = Projection * View * vec4(instance.lightPosition + Position.xyz * instance.lightRadius, 1);
	gl_Position = Projection * vec4(vLightPosition + Position.xyz * instance.lightRadius, 1);
}