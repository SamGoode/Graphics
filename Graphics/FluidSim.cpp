#include "FluidSim.h"



// Spawns random particle within bounding box
void FluidSimSPH::spawnRandomParticles(unsigned int spawnCount) {
	sim->spawnRandomParticles(spawnCount);
}


void FluidSimSPH::update(float deltaTime) {
	sim->update(deltaTime);
}

void FluidSimSPH::tickSimGPU() {
	sim->stepSim();
}