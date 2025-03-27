#version 410


in vec4 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;

in vec3 Ka;
in vec3 Kd;
in vec3 Ks;
in float S;

out vec4 gpassAlbedo;
out vec3 gpassDiffuse;
out vec4 gpassSpecular;

out vec3 gpassPosition;
out vec3 gpassNormal;

uniform sampler2D baseTexture;

void main() {
	vec3 textureColor = texture(baseTexture, vTexCoord).rgb;
	gpassAlbedo = vec4(Ka * textureColor, 1);
	gpassDiffuse = Kd * textureColor;
	gpassSpecular = vec4(Ks.rgb, S);

	gpassPosition = vPosition.xyz;
	gpassNormal = normalize(vNormal);
}