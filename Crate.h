#ifndef CRATE_H
#define CRATE_H

#include "GlbMesh.h"

// ---------------------------------------------------------------------------
// Crate
//
// Wraps the crate.glb asset.  Convenience constructor accepts world-space
// position, uniform scale, and an optional Y-axis rotation (radians).
//
// Example:
//   Crate crate(2.0f, 0.0f, -1.5f, 0.5f);   // pos + scale
//   crate.drawFilled(shaderID);
// ---------------------------------------------------------------------------
class Crate : public GlbMesh
{
public:
    // path     – path to crate.glb (default: "assets/crate.glb")
    // x,y,z    – world-space position
    // scale    – uniform scale factor
    // yawRad   – rotation around the Y axis in radians
    explicit Crate(float x = 0.f, float y = 0.f, float z = 0.f,
                   float scale  = 1.f,
                   float yawRad = 0.f,
                   const std::string& path = "assets/crate.glb")
        : GlbMesh(path)
    {
        Matrix<4,4> t = makeTranslation3D(x, y, z)
                      * makeRotationY(yawRad)
                      * makeScale3D(scale, scale, scale);
        setTransform(t);
    }
};

#endif // CRATE_H
