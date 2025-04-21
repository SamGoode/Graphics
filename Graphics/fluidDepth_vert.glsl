#version 430 core

#define MAX_PARTICLES 4096
#define MAX_PARTICLES_PER_CELL 16


layout(std140) uniform PVMatrices {
	mat4 View;
	mat4 Projection;
	mat4 ViewInverse;
	mat4 ProjectionInverse;
	vec4 CameraPos;
};

layout(binding = 1, std430) readonly restrict buffer FluidSimSSBO {
	uint particleCount;
	float smoothingRadius;
	vec2 padding;
	vec4 positions[MAX_PARTICLES];
	uint hashTable[MAX_PARTICLES];
	uint cellEntries[MAX_PARTICLES];
	uint cells[MAX_PARTICLES * MAX_PARTICLES_PER_CELL];
};

out vec4 vPosition;
//out float Depth;
out vec2 CenterOffset;

flat out float vSmoothingRadius;


void main() {
	const vec2 vertexOffsets[4] = vec2[4](vec2(-1, 1), vec2(-1, -1), vec2(1, -1), vec2(1, 1));

	vec4 center = View * vec4(positions[gl_InstanceID].xyz, 1);

	// Also offset towards camera by smoothing radius for depth testing reasons
//	SmoothingRadius = cellSize;
//	vPosition = center + vec4(vertexOffsets[gl_VertexID].xy * SmoothingRadius, SmoothingRadius, 0);

	vPosition = center + vec4(vertexOffsets[gl_VertexID].xy * smoothingRadius, smoothingRadius, 0);
	
	//Depth = length(vPosition.xyz);
	CenterOffset = vertexOffsets[gl_VertexID].xy;
	vSmoothingRadius = smoothingRadius;

	gl_Position = Projection * vPosition;
}