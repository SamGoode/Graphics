#version 410


struct instanceData {
	mat4 transform;
	vec3 baseColor;
    vec3 diffuseColor;
    vec3 specularColor;
	float specularExp;
};

in vec4 Position;
in vec4 Normal;
in vec2 TexCoord;

in instanceData instance;

out vec4 vPosition;
out vec3 vNormal;
out vec2 vTexCoord;

out vec3 Ka;
out vec3 Kd;
out vec3 Ks;
out float S;

uniform mat4 ProjectionView;

void main() {
	vPosition = instance.transform * Position;
	vNormal = (instance.transform * Normal).xyz;
	vTexCoord = TexCoord;

	Ka = instance.baseColor;
	Kd = instance.diffuseColor;
	Ks = instance.specularColor;
	S = instance.specularExp;

	gl_Position = ProjectionView * instance.transform * Position;
}