#ifndef BOLLARD_H
#define BOLLARD_H

#include "Shape.h"

// Bollard: a short cylinder body topped with a hemispherical domed cap.
// The dome is approximated by a stack of shrinking cylinder rings.
// Typical usage:
//   Bollard proto(0,0,0, COL);   proto.build();          // prototype
//   Bollard *b = new Bollard(0,0,0, COL);                // clone constructor
//   b->cloneBuffers(proto);                               // share GPU data
//   // then place b with a translation transform in the scene graph

class Bollard : public Shape
{
private:
    float cx, cy, cz;    // base-centre world position
    int   sectors;

    // Fixed proportions (can be tweaked here)
    static constexpr float BODY_RADIUS = 0.06f;
    static constexpr float BODY_HEIGHT = 0.55f;
    static constexpr float DOME_STACKS = 8;

public:
    Bollard(float cx, float cy, float cz,
            float r, float g, float b,
            int sectors = 16);

    void build() override;
};

#endif // BOLLARD_H
