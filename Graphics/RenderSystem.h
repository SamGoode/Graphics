#pragma once

#include "ECS.h"
#include "ECSComponents.h"
#include "glmAddon.h"

#include "Mesh.h"


mat4 getTransformMatrix(TransformComponent transformComponent) {
	mat4 scaleMat = glm::identity<mat4>();
	scaleMat *= vec4(transformComponent.scale, 1);

	mat4 rotMat = glm::mat4_cast(transformComponent.rotation);
	mat4 out = rotMat * scaleMat;
	out[3] += vec4(transformComponent.position, 0);

	return out;
}


class RenderSystem : public ECS::System {
public:
	void addMeshInstances(ECS::ECSManager& manager, Mesh* meshArray) {
		for (int i = 0; i < entityCount; i++) {
			MeshComponent meshComp = manager.getComponent<MeshComponent>(entities[i]);
			TransformComponent transformComp = manager.getComponent<TransformComponent>(entities[i]);
			MaterialComponent materialComp = manager.getComponent<MaterialComponent>(entities[i]);

			mat4 transform = getTransformMatrix(transformComp);
			MaterialProperties material = materialComp.material;

			meshArray[meshComp.meshID].addInstance(transform, material);
		}
	}
};