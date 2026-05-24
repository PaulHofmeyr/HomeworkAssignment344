#include "HoleNode.h"
#include "NodeUtils.h"
#include <cmath>

// Flag positions matching CourseLayout.cpp holeFlags[]
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

    // ── Flat layers from CourseLayout ─────────────────────
    holeNode->addChild(fnNode([&layout, i](){ layout.drawGreenbed(i); }));
    holeNode->addChild(fnNode([&layout, i](){ layout.drawGreen(i); }));
    // The flat disc flag (tiny red circle at flag base)
    holeNode->addChild(fnNode([&layout, i](){ layout.drawFlag(i); }));

    float fx = kFlags[i][0];
    float fz = kFlags[i][1];

    // ── 3-D Flag pole ──────────────────────────────────────
    // Pole: slim cylinder, 1.2 units tall
    auto* pole = makeShape<Cylinder>(
        fx, 0.6f, fz,
        0.025f, 1.2f, 8,
        0.80f, 0.80f, 0.80f);   // silver
    pole->build();
    holeNode->addChild(shapeNode(pole));

    // Banner: small red cuboid at top of pole
    auto* banner = makeShape<Cuboid>(
        fx + 0.10f, 1.12f, fz,
        0.20f, 0.12f, 0.03f,
        0.85f, 0.08f, 0.08f);   // red
    banner->build();
    holeNode->addChild(shapeNode(banner));

    // ── Hole cup (dark sunken cylinder) ───────────────────
    auto* cup = makeShape<Cylinder>(
        fx, 0.005f, fz,
        0.09f, 0.04f, 10,
        0.05f, 0.05f, 0.05f);   // near-black
    cup->build();
    holeNode->addChild(shapeNode(cup));

    return holeNode;
}
