#version 150

in vec4 Position;
in vec4 Normal;

//out vec4 vColor;
out vec3 vNormal;

uniform mat4 ProjectionViewModel;
uniform mat4 ModelTransform;

void main() {
	//vColor = vec4(vec3(0.8f), 1);
	vNormal = (ModelTransform * Normal).xyz;//(ModelTransform * vec4(Normal.xyz, 0)).xyz;
	gl_Position = ProjectionViewModel * Position;
}