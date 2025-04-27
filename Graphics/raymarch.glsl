#version 430 core

#include "common.h"


in vec2 vTexCoord;

uniform sampler2D fluidDepthPass;

layout(binding = PROJECTIONVIEW_UBO, std140) uniform PVMatrices {
	mat4 View;
	mat4 Projection;
	mat4 ViewInverse;
	mat4 ProjectionInverse;
	vec4 CameraPos;
};

layout(binding = FLUID_CONFIG_UBO, std140) uniform FluidConfig {
	vec4 boundsMin;
	vec4 boundsMax;
	vec4 gravity;
	float smoothingRadius;
	float restDensity;
	float stiffness;
	float nearStiffness;
	
	float timeStep;
	uint particleCount;
} config;

layout(binding = FLUID_DATA_SSBO, std430) readonly restrict buffer FluidData {
	vec4 positions[MAX_PARTICLES];
	vec4 previousPositions[MAX_PARTICLES];
	vec4 velocities[MAX_PARTICLES];
	vec4 pressureDisplacements[MAX_PARTICLES];
	float densities[MAX_PARTICLES];
	float nearDensities[MAX_PARTICLES];

	uint usedCells;
	uint hashTable[MAX_PARTICLES];
	uint cellEntries[MAX_PARTICLES];
	uint cells[];
} data;


layout(location = 0) out vec4 gpassAlbedoSpec;
layout(location = 1) out vec3 gpassPosition;
layout(location = 2) out vec3 gpassNormal;


// All 'point' parameters are in world space

ivec3 getCellCoords(vec3 point) {
	return ivec3(floor(point / config.smoothingRadius));
}

uint getCellHash(ivec3 cellCoords) {
	// Three large prime numbers (from the brain of Matthias Teschner)
	const uint p1 = 73856093;
	const uint p2 = 19349663; // Apparently this one isn't prime
	const uint p3 = 83492791;

	return ((p1 * uint(cellCoords.x)) ^ (p2 * uint(cellCoords.y)) ^ (p3 * uint(cellCoords.z))) % MAX_PARTICLES;
}

float densityKernel(float radius, float dist) {
	float value = 1 - (dist / radius);
	return value * value;
}

float sampleDensity(vec3 point) {
	ivec3 cellCoords = getCellCoords(point);

	float density = 0.0;
	for(int i = 0; i < 27; i++) {
		ivec3 offset = ivec3(i % 3, (i / 3) % 3, i / 9) - ivec3(1); // inefficient?
		ivec3 offsetCoords = ivec3(cellCoords) + offset;

		uint cellHash = getCellHash(offsetCoords);
		uint cellIndex = data.hashTable[cellHash];

		if(cellIndex == 0xFFFFFFFF) continue;

		uint entries = data.cellEntries[cellIndex];

		for(uint n = 0; n < entries; n++) {
			uint particleIndex = data.cells[(cellIndex * MAX_PARTICLES_PER_CELL) + n];
			vec3 toParticle = data.positions[particleIndex].xyz - point;
			float sqrDist = dot(toParticle, toParticle);
			
			if(sqrDist > config.smoothingRadius * config.smoothingRadius) continue;

			float dist = sqrt(sqrDist);
			density += densityKernel(config.smoothingRadius, dist);
		}
	}
	return density;
}


// Returns approximate distance between ray origin and density iso-surface
float raymarchDensity(vec3 rayOrigin, vec3 rayDir, int maxSteps, float stepLength, float isoDensity, float startDepth) {
	float accumulatedDensity = 0.0;
	for(int i = 0; i < maxSteps; i++) {
		vec3 stepPos = rayOrigin + rayDir * (stepLength * i + startDepth);

		float density = sampleDensity(stepPos);
		accumulatedDensity += density;

		if(accumulatedDensity > isoDensity) {
			return stepLength * i + startDepth;
		}
	}

	return -1.0;
}

vec3 densityGradient(vec3 point) {
	float dx = sampleDensity(point + vec3(0.001, 0, 0)) - sampleDensity(point + vec3(-0.001, 0, 0));
	float dy = sampleDensity(point + vec3(0, 0.001, 0)) - sampleDensity(point + vec3(0, -0.001, 0));
	float dz = sampleDensity(point + vec3(0, 0, 0.001)) - sampleDensity(point + vec3(0, 0, -0.001));
	
	return -vec3(dx, dy, dz);
}


// Raymarch settings
const int maxSteps = 64;
const float stepLength = 0.02;
const float isoDensity = 1.0;


// crappy color parameters for testing
const vec4 mercury = vec4(vec3(0.5), 1.0);
const vec4 water = vec4(vec3(0.1, 0.5, 0.8), 0.3);


void main() {
//	float sqrDist = dot(CenterOffset, CenterOffset);
//	if(sqrDist > 1) discard;
//
//	// distance^2 + height^2 = radius^2
//	// height = sqrt(radius^2 - distance^2)
//	float depthOffset = sqrt(1 - sqrDist);
//
//	float smoothingRadius = cellSize;
//	//vec3 depthOffsetPos = vPosition.xyz - vec3(0, 0, (1 - depthOffset) * smoothingRadius);
//	float minDepth = Depth + (1 - depthOffset) * smoothingRadius;
//	vec2 screenUVs = gl_FragCoord.xy / ScreenSize;
	//vec2 texCoord = gl_FragCoord.xy / vec2(1600, 900);//textureSize(fluidDepthPass, 0).xy;

	// vec2 minMaxDepth = texture(fluidDepthPass, vTexCoord).rg;
	// float minDepth = minMaxDepth.r;

	// vec2 screenUVs = vTexCoord;
	// vec2 ndc = screenUVs * 2 - 1;
	// vec3 vRayDirection = normalize((ProjectionInverse * vec4(ndc, 1, 1)).xyz);
	// vec3 depthOffsetPos = vRayDirection * minDepth;

	// gpassAlbedoSpec = water;
	// gpassPosition = depthOffsetPos;

	// float dz = 0.001;
	// float dx = (texture(fluidDepthPass, vTexCoord + vec2(dz, 0)).r - texture(fluidDepthPass, vTexCoord + vec2(-dz, 0)).r) * 0.5;
	// float dy = (texture(fluidDepthPass, vTexCoord + vec2(0, dz)).r - texture(fluidDepthPass, vTexCoord + vec2(0, -dz)).r) * 0.5;

	// gpassNormal = normalize(vec3(-dx, -dy, dz));
	// //gpassNormal = normalize(vec3(CenterOffset, depthOffset));

	// float clipZ = depthOffsetPos.z * Projection[2].z + Projection[3].z;
	// float ndcZ = clipZ / -depthOffsetPos.z;
	// gl_FragDepth = ndcZ * 0.5 + 0.5;


	vec2 screenUVs = vTexCoord;
	vec2 ndc = screenUVs * 2 - 1;

	vec2 minMaxDepth = texture(fluidDepthPass, vTexCoord).rg;

	vec3 vRayDirection = normalize((ProjectionInverse * vec4(ndc, 1, 1)).xyz);
	vec3 rayDirection = (ViewInverse * vec4(vRayDirection, 0)).xyz;	

	float minDepth = minMaxDepth.r;
	float maxDepth = minMaxDepth.g;

	float rayDistance = raymarchDensity(CameraPos.xyz, rayDirection, maxSteps, stepLength, isoDensity, minDepth);
	if(rayDistance == -1.0) discard;

	vec3 iso_vPos = vRayDirection * rayDistance;
	vec3 iso_pos = CameraPos.xyz + rayDirection * rayDistance;

	gpassAlbedoSpec = water;
	gpassPosition = iso_vPos;
	gpassNormal = (View * vec4(normalize(densityGradient(iso_pos)), 0)).xyz;

	float clipZ = iso_vPos.z * Projection[2].z + Projection[3].z;
	float ndcZ = clipZ / -iso_vPos.z;
	gl_FragDepth = ndcZ * 0.5 + 0.5;
}