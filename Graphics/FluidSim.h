#pragma once

#include "ModularFluids.h"

#include "glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/ext.hpp>
#include <glm/glm/fwd.hpp>

#include "common.h"

#include "SpatialGrid.h"
#include "UniformBuffer.h"
#include "ShaderStorageBuffer.h"
#include "Shader.h"


using glm::uvec2;
using glm::uvec3;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::uvec4;


// SPH Fluid simulation within a bounding box
class FluidSimSPH {
private:
	ISPH_Compute* sim;

public:
	FluidSimSPH() {}
	~FluidSimSPH() { ModularFluids::Destroy(sim); }

	void init(vec3 _position, vec3 _bounds, vec3 _gravity, float _particleRadius = 0.4f,
		float _restDensity = 1000.f, float _stiffness = 20.f, float _nearStiffness = 80.f) {
		
		sim = ModularFluids::Create();
		sim->init(_position, _bounds, _gravity, _particleRadius, _restDensity, _stiffness, _nearStiffness);
	}

	unsigned int getParticleCount() { return sim->getParticleCount(); }
	void spawnRandomParticles(unsigned int spawnCount);
	void clearParticles() { sim->clearParticles(); }

	void update(float deltaTime);
	void tickSimGPU();

	void bindConfigUBO(GLuint bindingIndex) { sim->bindConfigUBO(bindingIndex); }
	void bindParticleSSBO(GLuint bindingIndex) { sim->bindParticleSSBO(bindingIndex); }
};