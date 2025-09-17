#pragma once

#include <glm/glm/glm.hpp>
#include <glm/glm/ext.hpp>
#include <glm/glm/fwd.hpp>



// SPH Fluid simulation within a bounding box
class FluidSimSPH {
private:
	class ISPH_Compute* sim = nullptr;

public:
	FluidSimSPH() {}
	~FluidSimSPH();

	void init(glm::vec3 _position, glm::vec3 _bounds, glm::vec3 _gravity, float _particleRadius = 0.4f,
			float _restDensity = 1000.f, float _stiffness = 20.f, float _nearStiffness = 80.f);

	void update(float deltaTime);
	void tickSimGPU();

	void spawnRandomParticles(unsigned int spawnCount);
	unsigned int getParticleCount();
	void clearParticles();

	void bindConfigUBO(unsigned int bindingIndex);
	void bindParticleSSBO(unsigned int bindingIndex);

	ISPH_Compute* getSim();
};