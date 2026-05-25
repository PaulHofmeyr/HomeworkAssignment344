#ifndef BRIDGE_H
#define BRIDGE_H

#include "GlbMesh.h"

// ---------------------------------------------------------------------------
// Bridge
//
// Wraps the bridge.glb asset.
//
// Example:
//   Bridge bridge(0.f, 0.f, 0.f, 1.f, 1.5708f);  // 90° rotation
// ---------------------------------------------------------------------------
class Bridge : public GlbMesh
{
public:
    explicit Bridge(float x = 0.f, float y = 0.f, float z = 0.f,
                    float scale  = 1.f,
                    float yawRad = 0.f,
                    const std::string& path = "assets/bridge.glb")
        : GlbMesh(path)
    {
        Matrix<4,4> t = makeTranslation3D(x, y, z)
                      * makeRotationY(yawRad)
                      * makeScale3D(scale, scale, scale);
        setTransform(t);
    }
};

#endif // BRIDGE_H
