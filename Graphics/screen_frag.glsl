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

	float kernel[9] = float[](
		1, 1, 1,
		1, -8, 1,
		1, 1, 1
	);

	//vec3 sampleTex[9];
	float laplacian = 0.f;
	for(int i = 0; i < 9; i++) {
		vec4 color = texture(screenTexture, vTexCoord.st + offsets[i]);
		float average = (color.r, color.g, color.b) / 3.f;
		//sampleTex[i] = vec3(a);
		laplacian += average * kernel[i];
	}

	if(laplacian > 0.1f) {
		FragColor = vec4(0, 0, 0, 1);
		return;
	}

	FragColor = vec4(texture(screenTexture, vTexCoord));

	// Inverted
	//FragColor = vec4(1.f - texture(screenTexture, vTexCoord).rgb, 1);
}