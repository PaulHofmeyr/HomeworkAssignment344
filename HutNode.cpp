#include "HutNode.h"
#include "NodeUtils.h"

std::shared_ptr<SceneNode> buildHutNode(CourseLayout& layout)
{
    auto group = std::make_shared<SceneNode>();
    // Flat hut footprint from layout
    group->addChild(fnNode([&layout](){ layout.drawHut(); }));

    // ── 3-D structure ────────────────────────────────────────
    //  The hut sits roughly at world (0, 0, 0).
    //  6 hex-ring posts, radius 2.5, height 2.0

    for(int i = 0; i < 6; ++i){
        float a  = i * 3.14159f / 3.0f;
        float px = 2.5f * std::cos(a);
        float pz = 2.5f * std::sin(a);
        auto* post = makeShape<Cylinder>(
            px, 1.0f, pz,
            0.12f, 2.0f, 8,
            0.55f, 0.38f, 0.18f);  // timber
        post->build();
        group->addChild(shapeNode(post));
    }

    // Flat deck (thin wide cylinder = hex base)
    auto* deck = makeShape<Cylinder>(
        0.f, 0.05f, 0.f,
        2.6f, 0.08f, 6,
        0.65f, 0.48f, 0.25f);
    deck->build();
    group->addChild(shapeNode(deck));

    // Roof cone
    auto* roof = makeShape<Cone>(
        0.f, 2.1f, 0.f,
        2.8f, 1.4f, 6,
        0.20f, 0.20f, 0.20f);   // dark metal
    roof->build();
    group->addChild(shapeNode(roof));

    return group;
}
