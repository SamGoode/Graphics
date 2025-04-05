#version 410


float sdfBall(vec3 pos, vec3 ballPos, float ballRadius) {
	return length(ballPos - pos) - ballRadius;
}

const vec2 viewPortSize = vec2(1600, 900);

in vec2 vTexCoord;

uniform mat4 View;
uniform mat4 ViewInverse;
uniform mat4 Projection;
uniform mat4 ProjectionInverse;

//uniform vec3 CameraPos;
uniform vec3 BallPos;
uniform float BallRadius;

//uniform sampler2D albedoSpecPass;
//uniform sampler2D positionPass;
//uniform sampler2D normalPass;

out vec4 gpassAlbedoSpec;
out vec3 gpassPosition;
out vec3 gpassNormal;


void main() {
	//vec4 position = vec4(texture(positionPass, vTexCoord).xyz, 1);
	//float depth = -(View * position).z;
	vec2 ndc = (gl_FragCoord.xy / viewPortSize) * 2 - 1;
	vec3 rayDirection = normalize((ProjectionInverse * vec4(ndc, 1, 1)).xyz);
	rayDirection = (vec3(ndc.x * 0.736, ndc.y * 0.414, -1)) * 0.9999;

	//vec2 n = vec2(gl_FragCoord.xy - (viewPortSize.xy * 0.5f));
	//vec3 rayDirection = normalize(vec3(n, -1100.f));
	
	float stepSize = 0.f;
	for(int i = 0; i < 10; i++) {
		vec3 pos = rayDirection * stepSize;
		float dist = sdfBall(pos, BallPos, BallRadius);

		stepSize += dist;

		if(dist < 0.01f) {
			gpassAlbedoSpec = vec4(vec3(0, 0, 1), 1);
			gpassPosition = (ViewInverse * vec4(pos, 1)).xyz;

			vec4 screenSpacePos = Projection * vec4(pos, 1);
			//float z = (Projection * vec4(pos, 1)).z;
			float z = stepSize;
			gl_FragDepth = (1/z - 1/0.1f)/(1/1000.f - 1/0.1f);
			
			float dx = sdfBall(pos + vec3(0.001f, 0, 0), BallPos, BallRadius) - sdfBall(pos + vec3(-0.001f, 0, 0), BallPos, BallRadius);
			float dy = sdfBall(pos + vec3(0, 0.001f, 0), BallPos, BallRadius) - sdfBall(pos + vec3(0, -0.001f, 0), BallPos, BallRadius);
			float dz = sdfBall(pos + vec3(0, 0, 0.001f), BallPos, BallRadius) - sdfBall(pos + vec3(0, 0, -0.001f), BallPos, BallRadius);
			gpassNormal = (ViewInverse * vec4(normalize(vec3(dx, dy, dz)), 0)).xyz;

			return;
		}


		if(stepSize > 1000) {
			break;
		}
	}

	gl_FragDepth = 1;
//	gpassAlbedoSpec = texture(albedoSpecPass, vTexCoord);
//	gpassPosition = texture(positionPass, vTexCoord).xyz;
//	gpassNormal = texture(normalPass, vTexCoord).xyz;
}