#version 430 core

in vec4 vPosition;
//in float Depth;
in vec2 CenterOffset;

flat in float vSmoothingRadius;

layout(std140) uniform PVMatrices {
	mat4 View;
	mat4 Projection;
	mat4 ViewInverse;
	mat4 ProjectionInverse;
	vec4 CameraPos;
};

layout(location = 0) out vec2 fluidDepthPass;

layout(depth_greater) out float gl_FragDepth;

void main() {
	float sqrDist = dot(CenterOffset, CenterOffset);
	if(sqrDist > 1) discard;

	// distance^2 + height^2 = radius^2
	// height = sqrt(radius^2 - distance^2)
	float depthOffset = sqrt(1 - sqrDist);// * SmoothingRadius;

	// I have a suspicion this may be slightly incorrect
	//float minDepth = Depth - depthOffset;
	//float minDepth = Depth + (1 - depthOffset) * SmoothingRadius;
	//float maxDepth = Depth + depthOffset;

	vec3 depthOffsetPos = vPosition.xyz - vec3(0, 0, (1 - depthOffset) * vSmoothingRadius);
	float minDepth = length(depthOffsetPos);
	float maxDepth = 0;

	fluidDepthPass = vec2(minDepth, maxDepth);

	vec4 clipPos = (Projection * vec4(depthOffsetPos, 1));
	float ndcPosZ = clipPos.z / clipPos.w;
	gl_FragDepth = ndcPosZ * 0.5 + 0.5;
}