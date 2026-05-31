#ifndef PERIMETERFENCEMESH_H
#define PERIMETERFENCEMESH_H

#include "Shape.h"

// Single merged mesh for the whole map border fence (one draw call).
class PerimeterFenceMesh : public Shape
{
public:
    PerimeterFenceMesh();

    void build() override {}

    static PerimeterFenceMesh* create(
        float xMin, float xMax,
        float zMin, float zMax,
        float postSpacing = 2.0f);

private:
    void buildMesh(float xMin, float xMax, float zMin, float zMax, float spacing);

    void appendBox(std::vector<float>& buf,
                     float cx, float cy, float cz,
                     float hx, float hy, float hz,
                     float rotY) const;

    void appendPost(std::vector<float>& buf, float px, float pz) const;
};

#endif // PERIMETERFENCEMESH_H
