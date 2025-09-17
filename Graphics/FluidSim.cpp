#include "FluidSim.h"

#include "ModularFluids.h"


FluidSimSPH::~FluidSimSPH() { ModularFluids::Destroy(sim); }

void FluidSimSPH::init(glm::vec3 _position, glm::vec3 _bounds, glm::vec3 _gravity, float _particleRadius,
	float _restDensity, float _stiffness, float _nearStiffness) {

	sim = ModularFluids::Create();
	sim->init(_position, _bounds, _gravity, _particleRadius, _restDensity, _stiffness, _nearStiffness);
}

void FluidSimSPH::update(float deltaTime) { sim->update(deltaTime); }
void FluidSimSPH::tickSimGPU() { sim->stepSim(); }

// Spawns random particle within bounding box
void FluidSimSPH::spawnRandomParticles(unsigned int spawnCount) { sim->spawnRandomParticles(spawnCount); }
unsigned int FluidSimSPH::getParticleCount() { return sim->getParticleCount(); }
void FluidSimSPH::clearParticles() { sim->clearParticles(); }

void FluidSimSPH::bindConfigUBO(unsigned int bindingIndex) { sim->bindConfigUBO(bindingIndex); }
void FluidSimSPH::bindParticleSSBO(unsigned int bindingIndex) { sim->bindParticleSSBO(bindingIndex); }

ISPH_Compute* FluidSimSPH::getSim()
{
	return sim;
}
