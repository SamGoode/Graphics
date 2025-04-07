#version 430 core

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(std140) uniform PVMatrices {
	mat4 View;
	mat4 Projection;
	mat4 ViewInverse;
	mat4 ProjectionInverse;
};

layout(binding = 1, std430) buffer ssbo {
	uint count;
	float radius;
	vec4 positions[];
};


void main() {
	uint particleIndex = gl_GlobalInvocationID.x;

	vec4 Position = positions[particleIndex];
	vec4 vPosition = View * Position;

	positions[particleIndex] = vPosition;
}