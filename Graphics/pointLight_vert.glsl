#version 410

struct instanceData {
	vec3 lightPosition;
	float lightRadius;
	vec3 lightColor;
};

// might do the inbuilt vertex array trick later
in vec4 Position;

// per-instance data
in instanceData instance;

uniform mat4 View;
uniform mat4 ProjectionView;

out vec3 vLightPosition;
out float vLightRadius;
out vec3 vLightColor;


void main() {
	vLightPosition = (View * vec4(instance.lightPosition, 1)).xyz;
	vLightRadius = instance.lightRadius;
	vLightColor = instance.lightColor;
	
	gl_Position = ProjectionView * vec4(instance.lightPosition + Position.xyz * instance.lightRadius, 1);
}