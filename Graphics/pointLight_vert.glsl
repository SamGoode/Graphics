#version 410

// might do the inbuilt vertex array trick later
// per-vertex data
layout(location = 0) in vec3 Position;

// per-instance data
layout(location = 1) in struct instanceData {
	vec3 lightPosition;
	float lightRadius;
	vec3 lightColor;
} instance;


layout(std140) uniform PVMatrices {
	mat4 View;
	mat4 Projection;
	mat4 ViewInverse;
	mat4 ProjectionInverse;
};

out vec3 vLightPosition;
out float vLightRadius;
out vec3 vLightColor;


void main() {
	vLightPosition = (View * vec4(instance.lightPosition, 1)).xyz;
	vLightRadius = instance.lightRadius;
	vLightColor = instance.lightColor;
	
	//gl_Position = Projection * View * vec4(instance.lightPosition + Position.xyz * instance.lightRadius, 1);
	gl_Position = Projection * vec4(vLightPosition + Position.xyz * instance.lightRadius, 1);
}