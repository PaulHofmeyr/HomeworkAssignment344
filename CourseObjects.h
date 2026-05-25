#ifndef COURSEOBJECTS_H
#define COURSEOBJECTS_H

// ============================================================
//  CourseObjects.h
//
//  Ready-made builder functions for every course element.
//  Each function returns a SceneNode group you add as a child
//  of any hole or course node.
//
//  ALL of these use ProtoRegistry clones internally — no raw
//  Shape construction happens here.  Geometry is built once
//  at startup (ProtoRegistry::get().build()), every call here
//  is just transform + clone = very fast.
//
//  ┌─────────────────────┬──────────────────────────────────┐
//  │ Function            │ What it makes                    │
//  ├─────────────────────┼──────────────────────────────────┤
//  │ makeBoulderCluster  │ group of 3-5 boulders            │
//  │ makeLampPost        │ single lamp post + globe         │
//  │ makeFenceSection    │ posts + rails (hole decor)       │
//  │ makePerimeterFence  │ posts, rails, pickets on border   │
//  │ makeFlagpole        │ pole + banner                    │
//  │ makeHoleCup         │ dark cup marker                  │
//  │ makeShrubBed        │ cluster of shrubs                │
//  │ makeGrassClump      │ ornamental grass cluster         │
//  │ makeNativeTree      │ trunk + cone canopy              │
//  │ makeReedBed         │ group of reeds (pond edge)       │
//  │ makePathEdge        │ kerb strip of given length       │
//  └─────────────────────┴──────────────────────────────────┘
// ============================================================

#include <memory>
#include "SceneNode.h"

// ── Boulder cluster ──────────────────────────────────────────
//  cx, cz   = centre of the cluster in world space
//  spread   = radius the boulders are scattered within (metres)
//  count    = number of boulders (3-6 works well)
//  useSandstone = mix in warmer coloured boulders
std::shared_ptr<SceneNode> makeBoulderCluster(
    float cx, float cz,
    float spread = 0.8f,
    int   count  = 4,
    bool  useSandstone = false);

// ── Lamp post ────────────────────────────────────────────────
//  Places a single lamp post at (x, 0, z)
std::shared_ptr<SceneNode> makeLampPost(float x, float z);

// ── Fence section (posts + wooden rails, for holes) ──────────
std::shared_ptr<SceneNode> makeFenceSection(
    float startX, float startZ,
    float endX,   float endZ,
    int   posts = 3);

// ── Fence post line (posts + black rails + pickets) ──────────
std::shared_ptr<SceneNode> makeFencePostLine(
    float startX, float startZ,
    float endX,   float endZ,
    float spacing = 2.0f);

// ── Full map perimeter fence (posts, rails, pickets) ─────────
std::shared_ptr<SceneNode> makePerimeterFence(
    float xMin, float xMax,
    float zMin, float zMax,
    float spacing = 2.0f);

// ── Flagpole + banner ────────────────────────────────────────
std::shared_ptr<SceneNode> makeFlagpole(float x, float z);

// ── Hole cup ─────────────────────────────────────────────────
std::shared_ptr<SceneNode> makeHoleCup(float x, float z);

// ── Shrub bed ────────────────────────────────────────────────
//  cluster of shrubs centred at (cx, cz)
std::shared_ptr<SceneNode> makeShrubBed(
    float cx, float cz,
    int   count  = 3,
    float spread = 0.6f);

// ── Ornamental grass clump ───────────────────────────────────
std::shared_ptr<SceneNode> makeGrassClump(
    float cx, float cz,
    int   count  = 5,
    float spread = 0.4f);

// ── Native tree ──────────────────────────────────────────────
std::shared_ptr<SceneNode> makeNativeTree(
    float x, float z,
    float scale = 1.0f);

// ── Reed bed (aquatic edge) ───────────────────────────────────
std::shared_ptr<SceneNode> makeReedBed(
    float cx, float cz,
    int   count  = 6,
    float spread = 0.5f);

// ── Path edge kerb ────────────────────────────────────────────
//  startX/Z → endX/Z, places kerb strips along the line
std::shared_ptr<SceneNode> makePathEdge(
    float startX, float startZ,
    float endX,   float endZ);

#endif // COURSEOBJECTS_H
