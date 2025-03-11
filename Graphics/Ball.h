#pragma once

#include "Gizmos.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>

using glm::vec3;
using glm::vec4;

using aie::Gizmos;


struct Ball {
	float m_radius;
	float m_mass;

	vec3 m_pos;
	vec3 m_vel;
	vec3 m_acc;

	void update(float deltaTime) {
		m_pos += m_vel * deltaTime;
		m_vel += m_acc * deltaTime;
		m_acc = vec3(0);
	}

	void draw() {
		Gizmos::addSphere(m_pos, m_radius, 10, 10, vec4(0.8f, 0, 0, 1));
	}
};