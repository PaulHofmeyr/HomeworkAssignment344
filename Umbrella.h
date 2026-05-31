#ifndef UMBRELLA_H
#define UMBRELLA_H

#include "GlbMesh.h"

// ---------------------------------------------------------------------------
// Umbrella
//
// Wraps the umbrella.glb asset.
// ---------------------------------------------------------------------------
class Umbrella : public GlbMesh
{
public:
    explicit Umbrella(float x = 0.f, float y = 0.f, float z = 0.f,
                      float scale  = 1.f,
                      float yawRad = 0.f,
                      const std::string& path = "assets/umbrella.glb")
        : GlbMesh(path)
    {
        Matrix<4,4> t = makeTranslation3D(x, y, z)
                      * makeRotationY(yawRad)
                      * makeScale3D(scale, scale, scale);
        setTransform(t);
    }
};

#endif // UMBRELLA_H
