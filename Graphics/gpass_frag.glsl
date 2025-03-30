#version 410

in vec4 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;

in vec3 Ka;
in float S;

uniform sampler2D baseTexture;

out vec4 gpassAlbedoSpec;
out vec3 gpassPosition;
out vec3 gpassNormal;


void main() {
	vec3 textureColor = texture(baseTexture, vTexCoord).rgb;
	gpassAlbedoSpec = vec4(Ka * textureColor, S);

	gpassPosition = vPosition.xyz;
	gpassNormal = normalize(vNormal);
}