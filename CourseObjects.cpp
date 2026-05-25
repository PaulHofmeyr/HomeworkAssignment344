#include "CourseObjects.h"
#include "Prototype.h"
#include "NodeUtils.h"
#include "PerimeterFenceMesh.h"
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
    float angle = std::atan2(-dz, dx);

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
//  Perimeter fence edge — posts, top/bottom rails, pickets
// ============================================================
static int postsAlongLength(float len, float spacing)
{
    if(len < 1e-4f)
        return 1;
    return std::max(2, (int)std::ceil(len / spacing) + 1);
}

static std::shared_ptr<SceneNode> makePerimeterFenceEdge(
    float startX, float startZ,
    float endX,   float endZ,
    float spacing)
{
    auto& R = ProtoRegistry::get();
    auto group = std::make_shared<SceneNode>();

    constexpr float RAIL_BOT        = 0.18f;
    constexpr float RAIL_TOP        = 0.82f;
    constexpr float RAIL_UNIT_LEN   = 1.0f;
    constexpr int   PICKETS_PER_BAY = 18;

    float dx = endX - startX;
    float dz = endZ - startZ;
    float len = std::sqrt(dx * dx + dz * dz);
    if(len < 1e-4f)
        return group;

    int posts = postsAlongLength(len, spacing);

    // Posts along edge (corners at t=0 and t=1)
    for(int i = 0; i < posts; ++i)
    {
        float t = (posts > 1) ? (float)i / (float)(posts - 1) : 0.f;
        float px = startX + t * dx;
        float pz = startZ + t * dz;
        group->addChild(R.place(ProtoRegistry::FENCE_POST, px, 0.f, pz));
    }

    // Rails + pickets per bay: from one post to the next
    for(int i = 0; i < posts - 1; ++i)
    {
        float t0 = (float)i / (float)(posts - 1);
        float t1 = (float)(i + 1) / (float)(posts - 1);
        float sx = startX + t0 * dx;
        float sz = startZ + t0 * dz;
        float ex = startX + t1 * dx;
        float ez = startZ + t1 * dz;

        float segDx = ex - sx;
        float segDz = ez - sz;
        float segLen = std::sqrt(segDx * segDx + segDz * segDz);
        if(segLen < 1e-4f)
            continue;

        // Align local +X with segment direction in XZ
        float angle  = std::atan2(-segDz, segDx);
        float midX   = (sx + ex) * 0.5f;
        float midZ   = (sz + ez) * 0.5f;
        float scaleX = segLen / RAIL_UNIT_LEN;

        // Top & bottom rails: post to post
        group->addChild(R.place(ProtoRegistry::FENCE_RAIL_BLACK,
                                midX, RAIL_BOT, midZ,
                                scaleX, 1.f, 1.f, angle));
        group->addChild(R.place(ProtoRegistry::FENCE_RAIL_BLACK,
                                midX, RAIL_TOP, midZ,
                                scaleX, 1.f, 1.f, angle));

        // 18 vertical pickets evenly spaced between the two posts
        for(int k = 1; k <= PICKETS_PER_BAY; ++k)
        {
            float t = (float)k / (float)(PICKETS_PER_BAY + 1);
            float px = sx + t * segDx;
            float pz = sz + t * segDz;
            group->addChild(R.place(ProtoRegistry::FENCE_PICKET,
                                    px, RAIL_BOT, pz,
                                    1.f, 1.f, 1.f, angle));
        }
    }

    return group;
}

std::shared_ptr<SceneNode> makeFencePostLine(
    float startX, float startZ,
    float endX,   float endZ,
    float spacing)
{
    return makePerimeterFenceEdge(startX, startZ, endX, endZ, spacing);
}

// ============================================================
//  makePerimeterFence — rectangle along course map bounds
// ============================================================
std::shared_ptr<SceneNode> makePerimeterFence(
    float xMin, float xMax,
    float zMin, float zMax,
    float spacing)
{
    // One merged mesh instead of thousands of prototype clones (much faster).
    return shapeNode(PerimeterFenceMesh::create(xMin, xMax, zMin, zMax, spacing));
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
