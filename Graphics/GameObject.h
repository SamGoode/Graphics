#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Geometry.h"
#include "Registry.h"


using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::quat;


class GameObject : public Registry<GameObject> {
public:
    GameObject() {}
    virtual ~GameObject() {};
};


