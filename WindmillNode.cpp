#include "WindmillNode.h"
#include "NodeUtils.h"
#include <cmath>

WindmillResult buildWindmillNode()
{
    auto wmNode = std::make_shared<SceneNode>();

    // ── Position at hole 18 ───────────────────────────────
    wmNode->localTransform = makeTranslation3D(-16.337f, 0.0f, -14.542f);

    // ── Stone leg pillars (6 around base) ─────────────────
    for(int i = 0; i < 6; ++i){
        float a = i * 3.14159f / 3.0f;
        float px = 0.38f * std::cos(a);
        float pz = 0.38f * std::sin(a);
        auto* leg = makeShape<Cuboid>(
            px, 0.28f, pz,
            0.07f, 0.28f, 0.07f,
            0.78f, 0.70f, 0.55f);   // stone
        leg->build();
        wmNode->addChild(shapeNode(leg));
    }

    // ── Tower ─────────────────────────────────────────────
    auto* tower = makeShape<Cylinder>(
        0.f, 0.84f, 0.f,
        0.35f, 0.95f, 16,
        0.78f, 0.70f, 0.55f);   // stone
    tower->build();
    wmNode->addChild(shapeNode(tower));

    // ── Dark cap ring ──────────────────────────────────────
    auto* cap = makeShape<Cylinder>(
        0.f, 1.40f, 0.f,
        0.37f, 0.18f, 16,
        0.22f, 0.22f, 0.22f);   // dark
    cap->build();
    wmNode->addChild(shapeNode(cap));

    // ── Cone roof ─────────────────────────────────────────
    auto* roof = makeShape<Cone>(
        0.f, 1.49f, 0.f,
        0.37f, 0.40f, 16,
        0.15f, 0.15f, 0.15f);   // near-black
    roof->build();
    wmNode->addChild(shapeNode(roof));

    // ── Rotor group ───────────────────────────────────────
    //  main.cpp updates rotorNode->localTransform every frame
    //  with the pivot-spin-unpivot matrix.
    auto rotorNode = std::make_shared<SceneNode>();

    // Axle hub
    auto* axle = makeShape<Cylinder>(
        0.f, 1.48f, 0.43f,
        0.06f, 0.04f, 12,
        0.60f, 0.60f, 0.65f);   // metal grey
    axle->build();
    rotorNode->addChild(shapeNode(axle));

    // 4 blades (cross pattern)
    const float bLen = 0.35f, bW = 0.05f, bD = 0.015f;
    const float bY   = 1.48f, bZ = 0.43f;
    float bOffsets[4][2] = {
        { 0.f,    bLen/2.f},  // up
        { bLen/2.f, 0.f   },  // right
        { 0.f,   -bLen/2.f},  // down
        {-bLen/2.f, 0.f   }   // left
    };
    for(int i = 0; i < 4; ++i){
        bool vert = (i % 2 == 0);
        auto* blade = makeShape<Cuboid>(
            bOffsets[i][0], bY + bOffsets[i][1], bZ,
            vert ? bW : bLen,
            vert ? bLen : bW,
            bD,
            0.92f, 0.92f, 0.88f);   // off-white
        blade->build();
        rotorNode->addChild(shapeNode(blade));
    }

    SceneNode* rotorPtr = rotorNode.get();
    wmNode->addChild(rotorNode);

    return { wmNode, rotorPtr };
}
