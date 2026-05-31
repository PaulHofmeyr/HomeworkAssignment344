#include "HoleNodes.h"
#include "NodeUtils.h"
#include "Prototype.h"
#include "CourseObjects.h"
#include "TurfWall.h"

std::shared_ptr<SceneNode> buildHole16Node(CourseLayout& layout)
{
    auto node = std::make_shared<SceneNode>();
    constexpr int  IDX = 15;
    constexpr float fx = -12.772f, fz = 11.375f;

    node->addChild(fnNode([&layout](){ layout.drawGreenbed(IDX); }));
    node->addChild(fnNode([&layout](){ layout.drawGreen   (IDX); }));
    node->addChild(fnNode([&layout](){ layout.drawFlag    (IDX); }));
    node->addChild(shapeNode(TurfWall::createHole16()));

    node->addChild(makeFlagpole(fx, fz));
    node->addChild(makeHoleCup(fx, fz));
    node->addChild(makeBoulderCluster(fx, fz + 1.2f, 0.6f, 3));
    node->addChild(makeGrassClump(fx + 1.0f, fz - 0.8f, 4, 0.3f));
    return node;
}
