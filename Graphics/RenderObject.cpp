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

void RenderObject::initMesh() {
    switch (shape->getID()) {
    case 0:
        mesh.genCubeVerts();
        mesh.init();
        break;
    case 1:
        mesh.genSphereVerts();
        mesh.init();
        break;
    }
}

mat4 RenderObject::getTransform() {
    mat4 scale = glm::identity<mat4>();
    
    switch (shape->getID()) {
    case 0:
        vec3 dimensions = static_cast<Box*>(shape)->extents * 2.f;
        scale[0][0] = dimensions.x;
        scale[1][1] = dimensions.y;
        scale[2][2] = dimensions.z;
        break;
    case 1:
        scale *= vec4(vec3(static_cast<Sphere*>(shape)->radius), 1);
        break;
    }

    mat4 rotMat = glm::mat4_cast(rot);
    mat4 out = rotMat * scale;
    
    out[3] += vec4(pos, 0);

    return out;
}


