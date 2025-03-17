#include "RenderObject.h"


RenderObject::RenderObject(vec3 _pos, vec3 _eulerRot, Geometry* _geometry) {
    pos = _pos;

    vec3 euler = glm::radians(_eulerRot);
    quat qx = quat(cos(euler.x / 2), sin(euler.x / 2) * vec3(1, 0, 0));
    quat qy = quat(cos(euler.y / 2), sin(euler.y / 2) * vec3(0, 1, 0));
    quat qz = quat(cos(euler.z / 2), sin(euler.z / 2) * vec3(0, 0, 1));
    rot = normalize(qx * qy * qz);

    shape = _geometry;
}

