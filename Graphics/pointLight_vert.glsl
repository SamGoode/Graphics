#version 410


// might do the inbuilt vertex array trick later
in vec4 Position;

// per-instance data
layout(location = 1) in struct instanceData {
	vec3 lightPosition;
	float lightRadius;
	vec3 lightColor;
} instance;

uniform mat4 ProjectionView;

out vec3 vLightPosition;
out float vLightRadius;
out vec3 vLightColor;


void main() {
	vLightPosition = instance.lightPosition;
	vLightRadius = instance.lightRadius;
	vLightColor = instance.lightColor;
	
	gl_Position = ProjectionView * vec4(instance.lightPosition + Position.xyz * instance.lightRadius, 1);
}