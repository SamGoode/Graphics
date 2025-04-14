#version 430 core

#define FLT_MAX 3.402823466e+38

#define MAX_PARTICLES 1024

//in vec2 vTexCoord;
in vec4 vPosition;

layout(std140) uniform PVMatrices {
	mat4 View;
	mat4 Projection;
	mat4 ViewInverse;
	mat4 ProjectionInverse;
};

uniform vec2 ScreenSize;

layout(binding = 1, std430) readonly restrict buffer FluidSimSSBO {
	vec4 simPosition;
	vec4 simBounds;
	ivec4 gridBounds;
	uint particleCount;
	float particleRadius;
	float cellSize;
	float padding;
	vec4 positions[1024];
	ivec2 hashList[1024];
	ivec2 lookupTable[8192];
};


//float sdfBox(vec3 point, vec3 boxPos, vec3 halfExtents) {
//	vec3 boxToPoint = point - boxPos;
//	vec3 offsetCorner = abs(boxToPoint) - halfExtents;
//
//	return min(0, max(offsetCorner.x, max(offsetCorner.y, offsetCorner.z))) + length(max(offsetCorner, vec3(0)));
//}

float sdfSphere(vec3 point, vec3 spherePos, float sphereRadius) {
	return length(spherePos - point) - sphereRadius;
}

float sdf(vec3 point, uint particleIndex) {
	return sdfSphere(point, positions[particleIndex].xyz, particleRadius);
}

float smoothMin(float a, float b, float k) {
	k *= 6.0;
	float h = max(k - abs(a - b), 0.0)/k;
	return min(a, b) - h * h * h * k * (1.0/6.0);
}


bool isValidCell(ivec3 cellCoords) {
	return !(any(lessThan(cellCoords, ivec3(0))) || any(greaterThanEqual(cellCoords, ivec3(gridBounds.xyz))));
}

int getCellHash(ivec3 cellCoords) {
	return cellCoords.x + (cellCoords.y * gridBounds.x) + (cellCoords.z * gridBounds.x * gridBounds.y);
}

ivec3 getCellCoords(vec3 point) {
	return ivec3(floor(point / cellSize));
}

float densityKernel(float radius, float dist) {
	float value = 1 - (dist / radius);
	return value * value;
}

float sampleDensity(vec3 point) {
	vec3 localPoint = (ViewInverse * vec4(point, 1.0)).xyz - simPosition.xyz;

	ivec3 cellCoords = getCellCoords(localPoint);
	if(!isValidCell(cellCoords)) {
		return -1.0;
	}

	float density = 0.0;
	for(int i = 0; i < 27; i++) {
		ivec3 offset = ivec3(int(mod(i, 3)), int(mod((i / 3), 3)), i / 9) - ivec3(1);
		ivec3 offsetCoords = ivec3(cellCoords) + offset;

		if(!isValidCell(offsetCoords)) continue;

		int cellHash = getCellHash(offsetCoords);

		ivec2 indexLookup = ivec2(lookupTable[cellHash]);
		if(indexLookup.x == -1) { continue; } // Empty cell
	
		int startIndex = indexLookup.x;
		int endIndex = indexLookup.y;

		for(int n = startIndex; n < endIndex + 1; n++) {
			int particleIndex = hashList[n].x;
			vec3 toParticle = positions[particleIndex].xyz - localPoint;
			float sqrDist = dot(toParticle, toParticle);
			
			// cellSize is smoothing radius
			if(sqrDist > cellSize * cellSize) continue;

			float dist = sqrt(sqrDist);
			density += densityKernel(cellSize, dist);
		}
	}

	return density;
}

vec3 raymarchDensity(vec3 rayOrigin, vec3 rayDir, int maxSteps, float stepLength) {
	float accumulatedDensity = 0.0;
	for(int i = 0; i < maxSteps; i++) {
		// Apply a tiny little offset along the ray so it starts within the bounds of the box.
		vec3 stepPos = rayOrigin + rayDir * (stepLength * i + 0.0001);

		float density = sampleDensity(stepPos);
		if(density == -1.0) {
			break;
		}

		accumulatedDensity += density;
		if(accumulatedDensity > 0.5) {
			return stepPos;
		}
	}

	return vec3(0, 0, 1);
}

vec3 densityGradient(vec3 point) {
	float dx = sampleDensity(point + vec3(0.001, 0, 0)) - sampleDensity(point + vec3(-0.001, 0, 0));
	float dy = sampleDensity(point + vec3(0, 0.001, 0)) - sampleDensity(point + vec3(0, -0.001, 0));
	float dz = sampleDensity(point + vec3(0, 0, 0.001)) - sampleDensity(point + vec3(0, 0, -0.001));
	
	return -vec3(dx, dy, dz);
}

// Spatial grid hash attempt
//float sdfMin(vec3 point) {
//	vec3 localPoint = (ViewInverse * vec4(point, 1.0)).xyz - simPosition.xyz;
//
//	//ivec3 gridBounds = ivec3(ceil(simBounds.xyz / cellSize));
//	ivec3 cellCoords = getCellCoords(localPoint);
//
//	if(!isValidCell(cellCoords)) {
//		return cellSize * 0.5;
//	}
//
//	// Iterate through particles in neighbouring cells
//	float minDist = 1000.0;
//	for(int i = 0; i < 27; i++) {
//		ivec3 offset = ivec3(int(mod(i, 3)), int(mod((i / 3), 3)), i / 9) - ivec3(1);
//		ivec3 offsetCoords = ivec3(cellCoords) + offset;
//
//		if(!isValidCell(offsetCoords)) continue;
//
//		int cellHash = getCellHash(offsetCoords);
//
//		ivec2 indexLookup = ivec2(lookupTable[cellHash]);
//		if(indexLookup.x == -1) { continue; } // Empty cell
//	
//		int startIndex = indexLookup.x;
//		int endIndex = indexLookup.y;
//		for(int n = startIndex; n < endIndex + 1; n++) {
//			int particleIndex = hashList[n].x;
//			float dist = sdf(point, particleIndex);
//
//			minDist = smoothMin(minDist, dist, particleRadius);
//			if(minDist < -0.0) {
//				return -0.0;
//			}
//		}
//	}
//
//	return (minDist < 1000.0) ? minDist : cellSize * 0.5;
//}
//
//
//
//vec3 sdfGradient(vec3 point) {
//	float dx = sdfMin(point + vec3(0.000001, 0, 0)) - sdfMin(point + vec3(-0.000001, 0, 0));
//	float dy = sdfMin(point + vec3(0, 0.000001, 0)) - sdfMin(point + vec3(0, -0.000001, 0));
//	float dz = sdfMin(point + vec3(0, 0, 0.000001)) - sdfMin(point + vec3(0, 0, -0.000001));
//	
//	return vec3(dx, dy, dz);
//}
//
//
//vec3 getRaymarchedPos(vec3 rayOrigin, vec3 rayDir, int maxSteps, float maxDist) {
//	float dist = 0;
//	for(int i = 0; i < maxSteps && dist < maxDist; i++) {
//		vec3 stepPos = rayOrigin + rayDir * dist;
//
//		float stepSize = sdfMin(stepPos);
//		if(stepSize < 0.01) {
//			return stepPos;
//		}
//
//		dist += stepSize;
//	}
//
//	return vec3(0, 0, 1);
//}

layout(location = 0) out vec4 gpassAlbedoSpec;
layout(location = 1) out vec3 gpassPosition;
layout(location = 2) out vec3 gpassNormal;

const int maxSteps = 128;
//const float maxDist = 1000.0;
const float stepLength = 0.05f;

void main() {
	vec2 screenUVs = gl_FragCoord.xy / ScreenSize;
	vec2 ndc = screenUVs * 2 - 1;
	//vec2 ndc = vTexCoord * 2 - 1;
	vec3 rayOrigin = vPosition.xyz;
	vec3 rayDirection = normalize((ProjectionInverse * vec4(ndc, 1, 1)).xyz);

	//float dist = getRaymarchDist(rayOrigin, rayDirection, maxSteps, maxDist);
	//vec3 pos = rayOrigin + rayDirection * dist;
	//vec3 pos = getRaymarchedPos(rayOrigin, rayDirection, maxSteps, maxDist);
	vec3 pos = raymarchDensity(rayOrigin, rayDirection, maxSteps, stepLength);

	//gpassAlbedoSpec = vec4(vec3(0.5), 1.0); // Metallic look
	gpassAlbedoSpec = vec4(vec3(0.1, 0.5, 0.8), 0.3);
	gpassPosition = (vec4(pos, 1)).xyz;
	//gpassNormal = normalize(sdfGradient(pos).xyz);
	gpassNormal = normalize(densityGradient(pos));
	//gpassNormal = normalize(vec3(1, -1, 1));

	vec4 clipPos = (Projection * vec4(pos, 1));
	float ndcPosZ = clipPos.z / clipPos.w;

	gl_FragDepth = ndcPosZ * 0.5 + 0.5;
}