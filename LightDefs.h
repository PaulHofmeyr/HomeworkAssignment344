#ifndef LIGHTDEFS_H
#define LIGHTDEFS_H

// Shared plain structs — no GL, no Assimp, no other project headers.
// Include this anywhere you need Vec3 or SpotDef without pulling in all
// of Lighting.h.

#include <vector>

struct Vec3 { float x, y, z; };

struct SpotDef
{
    Vec3  pos;
    Vec3  dir;
    float innerRad   = 0.261f;
    float outerRad   = 0.436f;
    Vec3  ambient    = {0.01f, 0.01f, 0.01f};
    Vec3  diffuse    = {0.95f, 0.95f, 0.90f};
    Vec3  specular   = {0.50f, 0.50f, 0.50f};
    float constant   = 1.0f;
    float linear     = 0.027f;
    float quadratic  = 0.0028f;
    bool  onAtDusk   = true;
    bool  castShadow = true;
};

struct MapPointLight
{
    Vec3  pos;
    Vec3  ambient   = {0.f, 0.f, 0.f};
    Vec3  diffuse   = {0.6f, 0.5f, 0.3f};
    Vec3  specular  = {0.2f, 0.15f, 0.08f};
    float constant  = 1.f;
    float linear    = 0.14f;
    float quadratic = 0.07f;
    bool  castCubeShadow = true;
};

#endif // LIGHTDEFS_H