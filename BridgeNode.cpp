#include "BridgeNode.h"
#include "NodeUtils.h"

std::shared_ptr<SceneNode> buildBridgeNode(CourseLayout& layout)
{
    auto group = std::make_shared<SceneNode>();

    // Flat bridge polygons from the layout
    group->addChild(fnNode([&layout](){ layout.drawBridges(); }));

    // ── 3-D railings on each bridge ─────────────────────────
    //  Bridge 01 is roughly at (-3, 0, 12).
    //  Bridge 02 is roughly at ( 3, 0,  8).
    //  Each gets two thin Cuboid railings along its length.
    //
    //  Adjust the positions here to align with the flat polys.
    struct BridgeInfo { float x, z, len, rot; };
    static const BridgeInfo bridges[2] = {
        { -3.0f,  12.0f, 3.0f, 0.0f  },   // bridge 01
        {  3.0f,   8.0f, 2.5f, 0.3f  },   // bridge 02
    };

    for(auto& b : bridges){
        // Left railing
        auto* r1 = makeShape<Cuboid>(
            b.x - 0.5f, 0.15f, b.z,
            0.06f, 0.30f, b.len,
            0.55f, 0.38f, 0.18f);   // timber brown
        r1->build();
        group->addChild(shapeNode(r1));

        // Right railing
        auto* r2 = makeShape<Cuboid>(
            b.x + 0.5f, 0.15f, b.z,
            0.06f, 0.30f, b.len,
            0.55f, 0.38f, 0.18f);
        r2->build();
        group->addChild(shapeNode(r2));
    }

    return group;
}
