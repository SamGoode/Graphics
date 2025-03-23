#pragma once

#include "GameObject.h"
#include "Mesh.h"


class RenderObject : public GameObject, public Registry<RenderObject> {
public:
    vec3 pos = vec3(0);
    quat rot = quat(1, 0, 0, 0);

    vec4 color = vec4(0, 0, 0, 1);
    Geometry* shape = nullptr;
    Mesh mesh;

public:
    RenderObject() {}
    RenderObject(vec3 _pos, vec3 _eulerRot, Geometry* _geometry);
    virtual ~RenderObject() { delete shape; }

    void initMesh();
    mat4 getTransform();
    //virtual void draw();

    void setColor(vec4 _color) { color = _color; }
    int getID() { return shape->getID(); };
};