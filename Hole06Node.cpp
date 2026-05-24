#include "HoleNodes.h"
#include "NodeUtils.h"
#include "Prototype.h"
#include "CourseObjects.h"

std::shared_ptr<SceneNode> buildHole06Node(CourseLayout& layout)
{
    auto node = std::make_shared<SceneNode>();
    constexpr int  IDX = 5;
    constexpr float fx = 11.795f, fz = 14.483f;

    node->addChild(fnNode([&layout](){ layout.drawGreenbed(IDX); }));
    node->addChild(fnNode([&layout](){ layout.drawGreen   (IDX); }));
    node->addChild(fnNode([&layout](){ layout.drawFlag    (IDX); }));

    node->addChild(makeFlagpole(fx, fz));
    node->addChild(makeHoleCup(fx, fz));
    node->addChild(makeBoulderCluster(fx + 1.2f, fz - 0.5f, 0.7f, 4));
    node->addChild(makeShrubBed(fx - 1.0f, fz - 1.0f, 3, 0.6f));
    node->addChild(makeNativeTree(fx + 2.0f, fz + 1.0f));
    return node;
}
