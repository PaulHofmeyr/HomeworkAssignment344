#ifndef CYLINDER_H
#define CYLINDER_H

#include "Shape.h"

class Cylinder : public Shape
{
private:
    float cx, cy, cz;
    float radius;
    float height;
    int sectors;

public:
    Cylinder(float cx, float cy, float cz,
             float radius, float height,
             int sectors,
             float r, float g, float b);

    void build() override;
};

#endif // CYLINDER_H