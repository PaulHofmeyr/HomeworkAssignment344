#ifndef BOLLARD_H
#define BOLLARD_H

#include "GlbMesh.h"
#include "LightDefs.h"

// ---------------------------------------------------------------------------
// Bollard – wraps Bollard.glb and exposes its lamp-head position as a
// world-space Vec3 so Lighting can place a point light there.
//
// Raw GLB: origin at top of mesh (yMin=-7.428, yMax=0.16).
// At scale=0.105 total height ~0.80 m.  The lamp head is at yMax=0.16
// in model space → world Y = baseY + 0.16 * scale.
// ---------------------------------------------------------------------------
class Bollard : public GlbMesh
{
public:
    static constexpr float LAMP_MODEL_Y = 0.16f;  // raw GLB yMax

    explicit Bollard(float x = 0.f, float y = 0.f, float z = 0.f,
                     float scale  = 1.f,
                     float yawRad = 0.f,
                     const std::string& path = "assets/Bollard.glb")
        : GlbMesh(path),
          worldX_(x), worldY_(y), worldZ_(z), scale_(scale)
    {
        Matrix<4,4> t = makeTranslation3D(x, y, z)
                      * makeRotationY(yawRad)
                      * makeScale3D(scale, scale, scale);
        setTransform(t);
    }

    Vec3 getLightPos() const
    {
        return { worldX_, worldY_ + LAMP_MODEL_Y * scale_, worldZ_ };
    }

private:
    float worldX_, worldY_, worldZ_, scale_;
};

#endif // BOLLARD_H