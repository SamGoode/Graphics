#version 410

in vec2 vTexCoord;

out vec4 FragColor;

uniform sampler2D screenTexture;

const float offset = 1.f / 600.f;

void main() {
	vec2 offsets[9] = vec2[9](
		vec2(-offset, offset),
		vec2(0.f, offset),
		vec2(offset, offset),
		vec2(-offset, 0.f),
		vec2(0.f, 0.f),
		vec2(offset, 0.f),
		vec2(-offset, -offset),
		vec2(0.f, -offset),
		vec2(offset, -offset)
	);

	float laplacianSum = 0.f;
	for(int i = 0; i < 9; i++) {
		vec4 color = texture(screenTexture, vTexCoord.st + offsets[i]);
		float average = (color.r, color.g, color.b) / 3.f;
		laplacianSum += average;
	}

	vec4 originalColor = texture(screenTexture, vTexCoord);
	laplacianSum -= (originalColor.r, originalColor.g, originalColor.b) * 3.f;


	FragColor = originalColor;
	if(laplacianSum > 0.2f) {
		FragColor = vec4(0, 0, 0, 1);
	}
}