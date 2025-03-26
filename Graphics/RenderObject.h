#pragma once

#include "GameObject.h"
#include "Mesh.h"
#include "MaterialProperties.h"


class RenderObject : public GameObject, public Registry<RenderObject> {
public:
    vec3 pos = vec3(0);
    quat rot = quat(1, 0, 0, 0);

    int meshID = -1;
    MaterialProperties material = {
        .baseColor = vec3(0.2f),
        .diffuseColor = vec3(0.6f),
        .specularColor = vec3(0.8f),
        .specularExp = 32.f
    };

    Geometry* shape = nullptr;

public:
    RenderObject() {}
    RenderObject(vec3 _pos, vec3 _eulerRot, Geometry* _geometry);
    virtual ~RenderObject() { delete shape; }

    mat4 getTransform();
    int getID() { return shape->getID(); };

    MaterialProperties getMaterial() { return material; };

    void setColor(vec3 color) { material.baseColor = color; }
    void setDiffuseColor(vec3 diffuseColor) { material.diffuseColor = diffuseColor; }
    void setSpecularColor(vec3 specularColor) { material.specularColor = specularColor; }
    void setSpecularExp(float specularExp) { material.specularExp = specularExp; }
};