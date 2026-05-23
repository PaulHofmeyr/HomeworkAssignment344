#ifndef CONE_H
#define CONE_H

#include "Shape.h"

class Cone : public Shape
{
private:
    float cx, cy, cz;
    float radius;
    float height;
    int sectors;

public:
    Cone(float cx, float cy, float cz,
         float radius, float height,
         int sectors,
         float r, float g, float b);

    void build() override;
};

#endif // CONE_H