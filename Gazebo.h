#ifndef GAZEBO_H
#define GAZEBO_H

#include "GlbMesh.h"
#include "LightDefs.h"

// ---------------------------------------------------------------------------
// Gazebo – wraps Gazebo.glb and exposes a ceiling lamp position so Lighting
// can place a point light inside it.
//
// Raw GLB: yMin=0, yMax=15.7.  At scale=0.136 total height ~2.13 m.
// Lamp hangs at 75% of raw height → model Y = 11.77
// world lamp Y = worldY + 11.77 * scale  ≈  worldY + 1.60
// ---------------------------------------------------------------------------
class Gazebo : public GlbMesh
{
public:
    static constexpr float LAMP_MODEL_Y = 11.77f;  // 75% of raw yMax=15.7

    explicit Gazebo(float x = 0.f, float y = 0.f, float z = 0.f,
                    float scale  = 1.f,
                    float yawRad = 0.f,
                    const std::string& path = "assets/Gazebo.glb")
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

#endif // GAZEBO_H