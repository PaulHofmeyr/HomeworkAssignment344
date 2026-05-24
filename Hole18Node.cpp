#include "HoleNodes.h"
#include "NodeUtils.h"
#include "Prototype.h"
#include "CourseObjects.h"

std::shared_ptr<SceneNode> buildHole18Node(CourseLayout& layout)
{
    auto node = std::make_shared<SceneNode>();
    constexpr int  IDX = 17;
    constexpr float fx = -16.337f, fz = -14.542f;

    node->addChild(fnNode([&layout](){ layout.drawGreenbed(IDX); }));
    node->addChild(fnNode([&layout](){ layout.drawGreen   (IDX); }));
    node->addChild(fnNode([&layout](){ layout.drawFlag    (IDX); }));

    node->addChild(makeFlagpole(fx, fz));
    node->addChild(makeHoleCup(fx, fz));
    // Windmill hole — dramatic cluster
    node->addChild(makeBoulderCluster(fx + 1.0f, fz + 0.8f, 1.0f, 6, true));
    node->addChild(makeNativeTree(fx - 2.0f, fz + 2.0f));
    node->addChild(makeNativeTree(fx - 1.5f, fz + 3.0f, 1.3f));
    node->addChild(makeShrubBed(fx + 2.0f, fz - 0.5f, 4, 0.6f));
    node->addChild(makeLampPost(fx - 3.0f, fz));
    return node;
}
