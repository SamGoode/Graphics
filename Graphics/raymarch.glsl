#version 410


float sdfBall(vec3 pos, vec3 ballPos, float ballRadius) {
	return length(ballPos - pos) - ballRadius;
}

vec3 sdfGradient(vec3 pos, vec3 ballPos, float ballRadius) {
	float dx = sdfBall(pos + vec3(0.001f, 0, 0), ballPos, ballRadius) - sdfBall(pos + vec3(-0.001f, 0, 0), ballPos, ballRadius);
	float dy = sdfBall(pos + vec3(0, 0.001f, 0), ballPos, ballRadius) - sdfBall(pos + vec3(0, -0.001f, 0), ballPos, ballRadius);
	float dz = sdfBall(pos + vec3(0, 0, 0.001f), ballPos, ballRadius) - sdfBall(pos + vec3(0, 0, -0.001f), ballPos, ballRadius);
	
	return vec3(dx, dy, dz);
}

in vec2 vTexCoord;

uniform mat4 Projection;
uniform mat4 ProjectionInverse;

uniform vec3 BallPos;
uniform float BallRadius;

out vec4 gpassAlbedoSpec;
out vec3 gpassPosition;
out vec3 gpassNormal;


void main() {
	vec2 ndc = vTexCoord * 2 - 1;
	vec3 rayDirection = normalize((ProjectionInverse * vec4(ndc, 1, 1)).xyz);

	gl_FragDepth = 1;

	float stepSize = 0.f;
	for(int i = 0; i < 10; i++) {
		vec3 pos = rayDirection * stepSize;
		float dist = sdfBall(pos, BallPos, BallRadius);

		stepSize += dist;

		if(dist < 0.01f) {
			gpassAlbedoSpec = vec4(vec3(0, 0, 1), 1);
			gpassPosition = (vec4(pos, 1)).xyz;
			gpassNormal = normalize((vec4(sdfGradient(pos, BallPos, BallRadius), 0)).xyz);

			float z = (Projection * vec4(pos, 1)).z;
			
			gl_FragDepth = (1/z - 1/0.1f)/(1/1000.f - 1/0.1f);

			break;
		}


		if(stepSize > 1000) {
			break;
		}
	}

	
}