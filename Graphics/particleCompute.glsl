// Compute shader to transform particle positions into view space.
#version 430 core

#define MAX_PARTICLES 1024


layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(std140) uniform PVMatrices {
	mat4 View;
	mat4 Projection;
	mat4 ViewInverse;
	mat4 ProjectionInverse;
};

//layout(binding = 1, std430) buffer ssbo {
//	uint count;
//	float radius;
//	vec4 positions[];
//};

layout(binding = 1, std430) buffer FluidSimSSBO {
	vec4 simPosition;
	vec4 simBounds;
	uint particleCount;
	float particleRadius;
	float cellSize;
	float padding;
	vec4 positions[MAX_PARTICLES];
	uvec2 hashList[MAX_PARTICLES];
	uvec2 lookupTable[1024];
};


void main() {
	uint particleIndex = gl_GlobalInvocationID.x;

	vec4 Position = positions[particleIndex];
	vec4 vPosition = View * Position;

	//simPosition = View * simPosition;
	positions[particleIndex] = vPosition;
}