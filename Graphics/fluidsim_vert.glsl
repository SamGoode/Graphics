#version 430 core

#define MAX_PARTICLES 1024
#define MAX_CELL_COUNT 8192
#define MAX_PARTICLES_PER_CELL 16


layout(std140) uniform PVMatrices {
	mat4 View;
	mat4 Projection;
	mat4 ViewInverse;
	mat4 ProjectionInverse;
};

layout(binding = 1, std430) readonly restrict buffer FluidSimSSBO {
	vec4 simPosition;
	vec4 simBounds;
	ivec4 gridBounds;
	uint particleCount;
	float particleRadius;
	float cellSize;
	float padding;
	vec4 positions[MAX_PARTICLES];
//	ivec2 hashList[MAX_PARTICLES];
//	ivec2 lookupTable[MAX_CELL_COUNT];
	uint hashTable[MAX_PARTICLES];
	uint cellEntries[MAX_PARTICLES];
	uint cells[MAX_PARTICLES * MAX_PARTICLES_PER_CELL];
};

out vec4 Position;
out vec2 TexCoord;


void main() {
	const vec2 vertexOffsets[4] = vec2[4](vec2(-1, 1), vec2(-1, -1), vec2(1, -1), vec2(1, 1));

	vec4 center = View * vec4(positions[gl_InstanceID].xyz + simPosition.xyz, 1);

	float smoothingRadius = cellSize;
	vec4 vPosition = center + vec4(vertexOffsets[gl_VertexID].xy * smoothingRadius, 0, 0);
	Position = ViewInverse * vPosition;
	gl_Position = Projection * vPosition;

	TexCoord = vertexOffsets[gl_VertexID].xy * 0.5 + 0.5;

	//Position = vec4(positions[gl_VertexID].xyz + simPosition.xyz, 1);
	//gl_Position = Projection * View * Position;
	//gl_PointSize = 50.0/(gl_Position.z);
//	const vec3 vertices[8] = vec3[8](
//		vec3(0.5, 0.5, -0.5),
//		vec3(0.5, -0.5, -0.5),
//		vec3(-0.5, -0.5, -0.5),
//		vec3(-0.5, 0.5, -0.5),
//		vec3(0.5, 0.5, 0.5),
//		vec3(0.5, -0.5, 0.5),
//		vec3(-0.5, -0.5, 0.5),
//		vec3(-0.5, 0.5, 0.5)
//	);
//
//	const uint indices[36] = uint[36](
//		4, 6, 5, 4, 7, 6,
//		3, 1, 2, 3, 0, 1,
//		5, 0, 4, 5, 1, 0,
//		3, 2, 6, 3, 6, 7,
//		3, 4, 0, 3, 7, 4,
//		2, 1, 5, 2, 5, 6
//	);
//
//	vec3 vertexOffset = vertices[indices[gl_VertexID]];
//	vec3 boxCenter = (simPosition.xyz + simBounds.xyz * 0.5);
//	
//	Position = vec4(boxCenter + vertexOffset.xyz * simBounds.xyz, 1);
//	gl_Position = Projection * View * Position;
}