#include "HoleNodes.h"
#include "NodeUtils.h"
#include "Prototype.h"
#include "CourseObjects.h"

std::shared_ptr<SceneNode> buildHole15Node(CourseLayout& layout)
{
    auto node = std::make_shared<SceneNode>();
    constexpr int  IDX = 14;
    constexpr float fx = -18.242f, fz = 15.480f;

    node->addChild(fnNode([&layout](){ layout.drawGreenbed(IDX); }));
    node->addChild(fnNode([&layout](){ layout.drawGreen   (IDX); }));
    node->addChild(fnNode([&layout](){ layout.drawFlag    (IDX); }));

    node->addChild(makeFlagpole(fx, fz));
    node->addChild(makeHoleCup(fx, fz));
    node->addChild(makeBoulderCluster(fx + 1.0f, fz - 1.0f, 0.8f, 4));
    node->addChild(makeNativeTree(fx + 2.0f, fz, 1.2f));
    node->addChild(makeShrubBed(fx - 1.5f, fz + 0.5f, 3, 0.5f));
    return node;
}
