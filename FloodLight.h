#ifndef FLOODLIGHT_H
#define FLOODLIGHT_H

#include "GlbMesh.h"
#include "LightDefs.h"
#include <cmath>

// ---------------------------------------------------------------------------
// FloodLight – wraps FloodLight.glb and generates one SpotDef per pole,
// aimed from the lamp head toward the scene centre (0, 0, 0).
//
// Scale reasoning (used in Scene.cpp):
//   scale=1.5, worldY=1.5  →  base at ground (yMin_model=-1.0 → -1.0*1.5+1.5=0)
//                           →  lamp head at worldY + LAMP_MODEL_Y*scale = 1.5+1.5 = 3.0 m
// ---------------------------------------------------------------------------
class FloodLight : public GlbMesh
{
public:
    static constexpr float LAMP_MODEL_Y = 1.0f;  // raw GLB lamp head Y

    explicit FloodLight(float x = 0.f, float y = 0.f, float z = 0.f,
                        float scale  = 1.f,
                        float yawRad = 0.f,
                        const std::string& path = "assets/FloodLight.glb")
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

    // Appends one SpotDef per floodlight, aimed toward the scene centre.
    // The cone is wide enough to cover the course from the corner positions.
    void getSpotDefs(std::vector<SpotDef>& defs) const
    {
        Vec3 lp = getLightPos();

        // Direction: from lamp head toward scene centre (0, 0, 0)
        float dx = 0.f - lp.x;
        float dy = 0.f - lp.y;
        float dz = 0.f - lp.z;
        float len = std::sqrt(dx*dx + dy*dy + dz*dz);
        if (len < 1e-5f) len = 1.f;

        SpotDef d;
        d.pos       = lp;
        d.dir       = { dx/len, dy/len, dz/len };
        // Wide cone to cover the course (~3x5 units) from ~12 units away
        d.innerRad  = 0.40f;   // ~23 deg inner
        d.outerRad  = 0.60f;   // ~34 deg outer
        d.ambient   = {0.0f,  0.0f,  0.0f};   // no ambient bleed
        d.diffuse   = {0.85f, 0.82f, 0.75f};
        d.specular  = {0.45f, 0.45f, 0.40f};
        d.constant  = 1.0f;
        d.linear    = 0.014f;
        d.quadratic = 0.0007f;
        d.onAtDusk  = true;
        d.castShadow = (defs.size() < 2);
        defs.push_back(d);
    }

private:
    float worldX_, worldY_, worldZ_, scale_;
};

#endif // FLOODLIGHT_H