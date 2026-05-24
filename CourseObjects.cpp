#include "CourseObjects.h"
#include "Prototype.h"
#include "NodeUtils.h"
#include <cmath>
#include <cstdlib>   // for rand()

// ── Tiny deterministic "random" helper ───────────────────────
//  We use a simple seed so clusters look natural but are the
//  same every run (no jitter between frames).
static float seededRand(int seed, float lo, float hi)
{
    // Simple LCG — good enough for scatter offsets
    unsigned int s = (unsigned int)(seed * 2654435761u);
    float t = (float)(s & 0xFFFF) / 65535.0f;
    return lo + t * (hi - lo);
}

// ============================================================
//  makeBoulderCluster
// ============================================================
std::shared_ptr<SceneNode> makeBoulderCluster(
    float cx, float cz, float spread, int count, bool useSandstone)
{
    auto& R = ProtoRegistry::get();
    auto group = std::make_shared<SceneNode>();
    // group sits at identity — children carry individual positions

    for(int i = 0; i < count; ++i)
    {
        // Scatter offset — deterministic per index
        float ox  = seededRand(i * 7 + 1, -spread, spread);
        float oz  = seededRand(i * 7 + 3, -spread, spread);
        float rot = seededRand(i * 7 + 5,  0.f,    6.28f);

        // Alternate sizes: large, med, small to look natural
        ProtoRegistry::ID btype;
        if(useSandstone && i % 3 == 2)
            btype = ProtoRegistry::BOULDER_SANDSTONE;
        else if(i % 3 == 0)
            btype = ProtoRegistry::BOULDER_LARGE;
        else if(i % 3 == 1)
            btype = ProtoRegistry::BOULDER_MED;
        else
            btype = ProtoRegistry::BOULDER_SMALL;

        group->addChild(
            R.place(btype,
                    cx + ox, 0.f, cz + oz,  // position
                    1.f, 1.f, 1.f,           // scale
                    rot));                   // random Y rotation
    }
    return group;
}

// ============================================================
//  makeLampPost
// ============================================================
std::shared_ptr<SceneNode> makeLampPost(float x, float z)
{
    return ProtoRegistry::get().place(
        ProtoRegistry::LIGHT_POLE,
        x, 0.f, z);
}

// ============================================================
//  makeFenceSection
//  Posts are evenly spaced from start to end.
//  Two horizontal rails connect them at 0.3 and 0.7 height.
// ============================================================
std::shared_ptr<SceneNode> makeFenceSection(
    float startX, float startZ,
    float endX,   float endZ,
    int   posts)
{
    auto& R = ProtoRegistry::get();
    auto group = std::make_shared<SceneNode>();

    float dx = endX - startX;
    float dz = endZ - startZ;
    float len = std::sqrt(dx*dx + dz*dz);
    float angle = std::atan2(dx, dz);  // Y rotation to face along fence

    // Posts
    for(int i = 0; i < posts; ++i)
    {
        float t = (posts > 1) ? (float)i / (posts - 1) : 0.f;
        float px = startX + t * dx;
        float pz = startZ + t * dz;
        group->addChild(R.place(ProtoRegistry::FENCE_POST, px, 0.f, pz));
    }

    // Two rails: scaleX stretches the 0.5-unit rail prototype to 'len'
    float midX = (startX + endX) * 0.5f;
    float midZ = (startZ + endZ) * 0.5f;
    float scaleX = len / 0.5f;  // prototype rail halfW = 0.5

    // Lower rail at y = 0.3, upper at y = 0.7
    group->addChild(R.place(ProtoRegistry::FENCE_RAIL,
                             midX, 0.30f, midZ,
                             scaleX, 1.f, 1.f, angle));
    group->addChild(R.place(ProtoRegistry::FENCE_RAIL,
                             midX, 0.70f, midZ,
                             scaleX, 1.f, 1.f, angle));

    return group;
}

// ============================================================
//  makeFlagpole
// ============================================================
std::shared_ptr<SceneNode> makeFlagpole(float x, float z)
{
    return ProtoRegistry::get().place(
        ProtoRegistry::FLAGPOLE,
        x, 0.f, z);
}

// ============================================================
//  makeHoleCup
// ============================================================
std::shared_ptr<SceneNode> makeHoleCup(float x, float z)
{
    return ProtoRegistry::get().place(
        ProtoRegistry::HOLE_CUP,
        x, 0.f, z);
}

// ============================================================
//  makeShrubBed
// ============================================================
std::shared_ptr<SceneNode> makeShrubBed(
    float cx, float cz, int count, float spread)
{
    auto& R = ProtoRegistry::get();
    auto group = std::make_shared<SceneNode>();
    for(int i = 0; i < count; ++i)
    {
        float ox    = seededRand(i * 13 + 2, -spread, spread);
        float oz    = seededRand(i * 13 + 4, -spread, spread);
        float scale = seededRand(i * 13 + 6,  0.7f,   1.3f);
        group->addChild(
            R.place(ProtoRegistry::SHRUB,
                    cx + ox, 0.f, cz + oz,
                    scale, scale, scale));
    }
    return group;
}

// ============================================================
//  makeGrassClump
// ============================================================
std::shared_ptr<SceneNode> makeGrassClump(
    float cx, float cz, int count, float spread)
{
    auto& R = ProtoRegistry::get();
    auto group = std::make_shared<SceneNode>();
    for(int i = 0; i < count; ++i)
    {
        float ox    = seededRand(i * 11 + 1, -spread, spread);
        float oz    = seededRand(i * 11 + 3, -spread, spread);
        float scaleY = seededRand(i * 11 + 5, 0.8f, 1.6f);
        float rot   = seededRand(i * 11 + 7, 0.f,  6.28f);
        group->addChild(
            R.place(ProtoRegistry::ORNAMENTAL_GRASS,
                    cx + ox, 0.f, cz + oz,
                    1.f, scaleY, 1.f, rot));
    }
    return group;
}

// ============================================================
//  makeNativeTree
// ============================================================
std::shared_ptr<SceneNode> makeNativeTree(float x, float z, float scale)
{
    return ProtoRegistry::get().place(
        ProtoRegistry::NATIVE_TREE,
        x, 0.f, z,
        scale, scale, scale);
}

// ============================================================
//  makeReedBed
// ============================================================
std::shared_ptr<SceneNode> makeReedBed(
    float cx, float cz, int count, float spread)
{
    auto& R = ProtoRegistry::get();
    auto group = std::make_shared<SceneNode>();
    for(int i = 0; i < count; ++i)
    {
        float ox    = seededRand(i * 17 + 2, -spread, spread);
        float oz    = seededRand(i * 17 + 4, -spread, spread);
        float scaleY = seededRand(i * 17 + 6,  0.7f,  1.4f);
        float rot   = seededRand(i * 17 + 8,  0.f,   6.28f);
        group->addChild(
            R.place(ProtoRegistry::REED,
                    cx + ox, 0.f, cz + oz,
                    1.f, scaleY, 1.f, rot));
    }
    return group;
}

// ============================================================
//  makePathEdge
//  Tiles kerb strip prototypes (each 1.0 unit long) end-to-end
//  from start to end.
// ============================================================
std::shared_ptr<SceneNode> makePathEdge(
    float startX, float startZ,
    float endX,   float endZ)
{
    auto& R = ProtoRegistry::get();
    auto group = std::make_shared<SceneNode>();

    float dx    = endX - startX;
    float dz    = endZ - startZ;
    float len   = std::sqrt(dx*dx + dz*dz);
    float angle = std::atan2(dx, dz);
    float scaleX = len / 0.5f; // prototype halfW = 0.5, total width = 1.0

    float midX = (startX + endX) * 0.5f;
    float midZ = (startZ + endZ) * 0.5f;

    group->addChild(
        R.place(ProtoRegistry::PATH_EDGE,
                midX, 0.f, midZ,
                scaleX, 1.f, 1.f,
                angle));
    return group;
}
