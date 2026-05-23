#ifndef TRIANGULARPRISM_H
#define TRIANGULARPRISM_H

#include "Shape.h"

class TriangularPrism : public Shape
{
private:
    float cx, cy, cz;
    float hw, hh, tz;

public:
    TriangularPrism(float cx, float cy, float cz,
                    float hw, float hh, float tz,
                    float r, float g, float b);

    void build() override;
};

#endif // TRIANGULARPRISM_H