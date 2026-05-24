#include "HoleNode.h"
#include "NodeUtils.h"
#include "Prototype.h"
#include "CourseObjects.h"
#include <cmath>

// ============================================================
//  HoleNode.cpp
//
//  Builds one complete golf hole as a SceneNode tree.
//  Uses ProtoRegistry clones for all 3-D objects — no raw
//  Shape construction here.  Geometry is built once at startup.
//
//  STRUCTURE
//  ─────────
//  holeNode
//  ├── greenbedLayer   (flat polys from CourseLayout)
//  ├── greenLayer      (flat poly from CourseLayout)
//  ├── flatFlagDisc    (flat disc from CourseLayout)
//  ├── flagpole        (ProtoRegistry::FLAGPOLE clone)
//  ├── holeCup         (ProtoRegistry::HOLE_CUP clone)
//  └── obstacle group  (per-hole; uses CourseObjects helpers)
//
//  TO ADD AN OBSTACLE TO A SPECIFIC HOLE
//  ──────────────────────────────────────
//  Find the switch(i) block below and add your object inside
//  the case for that hole number (0-indexed: hole 01 = case 0).
//
//  AVAILABLE HELPERS  (from CourseObjects.h)
//  ──────────────────────────────────────────
//  makeBoulderCluster(cx, cz, spread, count, sandstone?)
//  makeLampPost(x, z)
//  makeFenceSection(x0,z0, x1,z1, posts)
//  makeShrubBed(cx, cz, count, spread)
//  makeGrassClump(cx, cz, count, spread)
//  makeNativeTree(x, z, scale)
//  makeReedBed(cx, cz, count, spread)
//  makePathEdge(x0,z0, x1,z1)
// ============================================================

static const float kFlags[18][2] = {
    { 2.515f, -21.754f},  // hole 01
    {15.116f, -20.640f},  // hole 02
    {18.535f, -17.767f},  // hole 03
    {11.648f,  -6.684f},  // hole 04
    {17.118f,   4.984f},  // hole 05
    {11.795f,  14.483f},  // hole 06
    { 8.034f,  -2.580f},  // hole 07
    {-0.855f, -14.952f},  // hole 08
    {-6.227f, -19.584f},  // hole 09
    {-9.890f,  -1.114f},  // hole 10
    {-2.759f,  11.023f},  // hole 11
    { 2.076f,  19.643f},  // hole 12
    { 5.086f,  15.194f},  // hole 13
    {-2.857f,  22.926f},  // hole 14
    {-18.242f, 15.480f},  // hole 15
    {-12.772f, 11.375f},  // hole 16
    {-17.753f,  0.997f},  // hole 17
    {-16.337f, -14.542f}  // hole 18
};

std::shared_ptr<SceneNode> buildHoleNode(CourseLayout& layout, int i)
{
    auto holeNode = std::make_shared<SceneNode>();
    auto& R = ProtoRegistry::get();

    float fx = kFlags[i][0];
    float fz = kFlags[i][1];

    // ── Flat map layers ───────────────────────────────────────
    holeNode->addChild(fnNode([&layout, i](){ layout.drawGreenbed(i); }));
    holeNode->addChild(fnNode([&layout, i](){ layout.drawGreen(i); }));
    holeNode->addChild(fnNode([&layout, i](){ layout.drawFlag(i); }));

    // ── 3-D flag pole (prototype clone) ──────────────────────
    holeNode->addChild(makeFlagpole(fx, fz));

    // ── 3-D hole cup (prototype clone) ───────────────────────
    holeNode->addChild(makeHoleCup(fx, fz));

    // ================================================================
    //  PER-HOLE OBSTACLES
    //  Add your objects inside the matching case below.
    //  Each case is one hole — fx/fz is the cup centre.
    //  Offset from fx/fz to place objects relative to the cup.
    // ================================================================
    switch(i)
    {
    case 0:  // ── HOLE 01 ──────────────────────────────────────
        // Boulder cluster to the left of the approach
        holeNode->addChild(makeBoulderCluster(fx - 1.2f, fz + 0.5f, 0.7f, 4));
        // Shrubs at the back edge
        holeNode->addChild(makeShrubBed(fx + 0.8f, fz + 1.0f, 3, 0.5f));
        // Lamp post by the tee
        holeNode->addChild(makeLampPost(fx - 2.0f, fz - 1.5f));
        break;

    case 1:  // ── HOLE 02 ──────────────────────────────────────
        holeNode->addChild(makeBoulderCluster(fx + 1.0f, fz - 0.8f, 0.6f, 3));
        holeNode->addChild(makeGrassClump(fx - 0.5f, fz + 1.2f, 4, 0.4f));
        break;

    case 2:  // ── HOLE 03 ──────────────────────────────────────
        holeNode->addChild(makeBoulderCluster(fx - 0.8f, fz + 0.8f, 0.8f, 5, true));
        holeNode->addChild(makeShrubBed(fx + 1.5f, fz, 2, 0.4f));
        break;

    case 3:  // ── HOLE 04 ──────────────────────────────────────
        holeNode->addChild(makeBoulderCluster(fx, fz - 1.0f, 0.6f, 3));
        holeNode->addChild(makeGrassClump(fx + 1.0f, fz + 0.5f, 5, 0.3f));
        holeNode->addChild(makeLampPost(fx + 2.5f, fz));
        break;

    case 4:  // ── HOLE 05 ──────────────────────────────────────
        holeNode->addChild(makeBoulderCluster(fx - 1.0f, fz - 1.0f, 0.9f, 4, true));
        holeNode->addChild(makeFenceSection(fx - 2.f, fz + 1.f, fx + 1.f, fz + 1.f, 4));
        break;

    case 5:  // ── HOLE 06 ──────────────────────────────────────
        holeNode->addChild(makeBoulderCluster(fx + 1.2f, fz - 0.5f, 0.7f, 4));
        holeNode->addChild(makeShrubBed(fx - 1.0f, fz - 1.0f, 3, 0.6f));
        holeNode->addChild(makeNativeTree(fx + 2.0f, fz + 1.0f));
        break;

    case 6:  // ── HOLE 07 ──────────────────────────────────────
        holeNode->addChild(makeBoulderCluster(fx, fz + 1.2f, 0.6f, 3));
        holeNode->addChild(makeGrassClump(fx - 1.2f, fz, 4, 0.4f));
        break;

    case 7:  // ── HOLE 08 ──────────────────────────────────────
        holeNode->addChild(makeBoulderCluster(fx - 1.0f, fz + 0.8f, 0.8f, 5));
        holeNode->addChild(makeLampPost(fx + 1.5f, fz - 1.0f));
        holeNode->addChild(makeShrubBed(fx + 0.5f, fz + 1.5f, 2, 0.5f));
        break;

    case 8:  // ── HOLE 09 ──────────────────────────────────────
        holeNode->addChild(makeBoulderCluster(fx + 0.8f, fz + 0.8f, 0.7f, 4, true));
        holeNode->addChild(makeGrassClump(fx - 1.0f, fz - 0.5f, 5, 0.3f));
        break;

    case 9:  // ── HOLE 10 ──────────────────────────────────────
        holeNode->addChild(makeBoulderCluster(fx - 1.2f, fz - 1.0f, 0.9f, 4));
        holeNode->addChild(makeNativeTree(fx - 2.5f, fz, 0.8f));
        holeNode->addChild(makeNativeTree(fx - 3.0f, fz + 1.0f));
        break;

    case 10: // ── HOLE 11 ──────────────────────────────────────
        holeNode->addChild(makeBoulderCluster(fx + 1.0f, fz, 0.6f, 3));
        holeNode->addChild(makeShrubBed(fx - 0.8f, fz + 1.2f, 4, 0.6f));
        holeNode->addChild(makeLampPost(fx + 2.0f, fz + 1.5f));
        break;

    case 11: // ── HOLE 12 ──────────────────────────────────────
        holeNode->addChild(makeBoulderCluster(fx - 0.8f, fz - 0.8f, 0.8f, 4, true));
        holeNode->addChild(makeFenceSection(fx + 1.5f, fz - 1.f, fx + 1.5f, fz + 1.5f, 3));
        break;

    case 12: // ── HOLE 13 ──────────────────────────────────────
        holeNode->addChild(makeBoulderCluster(fx + 1.2f, fz + 1.0f, 0.7f, 5));
        holeNode->addChild(makeGrassClump(fx - 1.0f, fz, 4, 0.4f));
        break;

    case 13: // ── HOLE 14 ──────────────────────────────────────
        holeNode->addChild(makeBoulderCluster(fx - 1.5f, fz + 0.5f, 1.0f, 5, true));
        holeNode->addChild(makeReedBed(fx + 1.0f, fz + 1.5f, 5, 0.4f));
        holeNode->addChild(makeLampPost(fx + 2.5f, fz - 0.5f));
        break;

    case 14: // ── HOLE 15 ──────────────────────────────────────
        holeNode->addChild(makeBoulderCluster(fx + 1.0f, fz - 1.0f, 0.8f, 4));
        holeNode->addChild(makeNativeTree(fx + 2.0f, fz, 1.2f));
        holeNode->addChild(makeShrubBed(fx - 1.5f, fz + 0.5f, 3, 0.5f));
        break;

    case 15: // ── HOLE 16 ──────────────────────────────────────
        holeNode->addChild(makeBoulderCluster(fx, fz + 1.2f, 0.6f, 3));
        holeNode->addChild(makeGrassClump(fx + 1.0f, fz - 0.8f, 4, 0.3f));
        break;

    case 16: // ── HOLE 17 ──────────────────────────────────────
        holeNode->addChild(makeBoulderCluster(fx - 1.0f, fz + 0.8f, 0.8f, 4, true));
        holeNode->addChild(makeFenceSection(fx - 2.f, fz - 1.f, fx + 0.5f, fz - 1.f, 3));
        holeNode->addChild(makeLampPost(fx + 1.5f, fz + 1.5f));
        break;

    case 17: // ── HOLE 18 ──────────────────────────────────────
        // Windmill hole — more dramatic boulder cluster
        holeNode->addChild(makeBoulderCluster(fx + 1.0f, fz + 0.8f, 1.0f, 6, true));
        holeNode->addChild(makeNativeTree(fx - 2.0f, fz + 2.0f));
        holeNode->addChild(makeNativeTree(fx - 1.5f, fz + 3.0f, 1.3f));
        holeNode->addChild(makeShrubBed(fx + 2.0f, fz - 0.5f, 4, 0.6f));
        holeNode->addChild(makeLampPost(fx - 3.0f, fz));
        break;

    default: break;
    }

    return holeNode;
}
