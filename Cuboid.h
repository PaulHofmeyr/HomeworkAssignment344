#ifndef CUBOID_H
#define CUBOID_H

#include "Shape.h"

class Cuboid : public Shape
{
private:
    float cx, cy, cz;
    float hw, hh, hd;

public:
    Cuboid(float cx, float cy, float cz,
           float hw, float hh, float hd,
           float r, float g, float b);

    void build() override;
};

#endif // CUBOID_H