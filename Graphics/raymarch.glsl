#version 430

#define MAX_PARTICLES 128

in vec2 vTexCoord;

layout(std140) uniform PVMatrices {
	mat4 View;
	mat4 Projection;
	mat4 ViewInverse;
	mat4 ProjectionInverse;
};

layout(binding = 1, std430) readonly buffer ssbo {
	uint count;
	float radius;
	vec3 positions[]; // discards w value
};

out vec4 gpassAlbedoSpec;
out vec3 gpassPosition;
out vec3 gpassNormal;


float sdfSphere(vec3 point, vec3 spherePos, float sphereRadius) {
	return length(spherePos - point) - sphereRadius;
}

float sdf(vec3 point, uint particleIndex) {
	return sdfSphere(point, positions[particleIndex], radius);
}

float smoothMin(float a, float b, float k) {
	k *= 6.0;
	float h = max(k - abs(a - b), 0.0)/k;
	return min(a, b) - h * h * h * k * (1.0/6.0);
}

float sdfMin(vec3 point) {
	float minDist = sdf(point, 0);

	for(uint particleIndex = 1; particleIndex < count; particleIndex++) {
		float dist = sdf(point, particleIndex);

		minDist = smoothMin(minDist, dist, radius/4.0);
	}

	return minDist;
}

vec3 sdfGradient(vec3 point) {
	float dx = sdfMin(point + vec3(0.001, 0, 0)) - sdfMin(point + vec3(-0.001, 0, 0));
	float dy = sdfMin(point + vec3(0, 0.001, 0)) - sdfMin(point + vec3(0, -0.001, 0));
	float dz = sdfMin(point + vec3(0, 0, 0.001)) - sdfMin(point + vec3(0, 0, -0.001));
	
	return vec3(dx, dy, dz);
}

vec3 sdfGradient(vec3 point, uint particleIndex) {
	float dx = sdf(point + vec3(0.001, 0, 0), particleIndex) - sdf(point + vec3(-0.001, 0, 0), particleIndex);
	float dy = sdf(point + vec3(0, 0.001, 0), particleIndex) - sdf(point + vec3(0, -0.001, 0), particleIndex);
	float dz = sdf(point + vec3(0, 0, 0.001), particleIndex) - sdf(point + vec3(0, 0, -0.001), particleIndex);
	
	return vec3(dx, dy, dz);
}

float getRaymarchDist(vec3 rayDir, int steps, out uint particleIndex) {
	float dist = 0;
	for(int i = 0; i < steps; i++) {
		float stepSize = sdfMin(rayDir * dist);
		if(stepSize < 0.05) {
			return dist;
		}

		dist += stepSize;
	}

	return -1;
}

const int stepCount = 32;

void main() {
	vec2 ndc = vTexCoord * 2 - 1;
	vec3 rayDirection = normalize((ProjectionInverse * vec4(ndc, 1, 1)).xyz);

	uint particleIndex;
	float dist = getRaymarchDist(rayDirection, stepCount, particleIndex);
	vec3 pos = rayDirection * dist;
	
	//gpassAlbedoSpec = vec4(vec3(0.5), 1.0); // Metallic look
	gpassAlbedoSpec = vec4(vec3(0.1, 0.5, 0.8), 0.5);
	gpassPosition = (vec4(pos, 1)).xyz;
	gpassNormal = normalize(sdfGradient(pos).xyz);

	vec4 clipPos = (Projection * vec4(pos, 1));
	float ndcPosZ = clipPos.z / clipPos.w;

	gl_FragDepth = ndcPosZ * 0.5 + 0.5;
}