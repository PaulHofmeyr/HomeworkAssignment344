#ifndef WAVE_H
#define WAVE_H

#include "GlbMesh.h"

// ---------------------------------------------------------------------------
// Wave
//
// Wraps the wave.glb asset.
// ---------------------------------------------------------------------------
class Wave : public GlbMesh
{
public:
    explicit Wave(float x = 0.f, float y = 0.f, float z = 0.f,
                  float scale  = 1.f,
                  float yawRad = 0.f,
                  const std::string& path = "assets/wave.glb")
        : GlbMesh(path)
    {
        Matrix<4,4> t = makeTranslation3D(x, y, z)
                      * makeRotationY(yawRad)
                      * makeScale3D(scale, scale, scale);
        setTransform(t);
    }
};

#endif // WAVE_H
