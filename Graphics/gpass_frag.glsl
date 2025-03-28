#version 410


in vec4 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;

in vec3 Ka;
//in vec3 Kd;
//in vec3 Ks;
in float S;

//out vec3 gpassDiffuse;
//out vec4 gpassSpecular;

out vec4 gpassAlbedoSpec;
out vec3 gpassPosition;
out vec3 gpassNormal;

uniform sampler2D baseTexture;

void main() {
	vec3 textureColor = texture(baseTexture, vTexCoord).rgb;
	gpassAlbedoSpec = vec4(Ka * textureColor, S);

	gpassPosition = vPosition.xyz;
	gpassNormal = normalize(vNormal);
}