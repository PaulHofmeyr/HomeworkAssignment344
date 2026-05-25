#include "HoleNodes.h"
#include "NodeUtils.h"
#include "Prototype.h"
#include "CourseObjects.h"
#include "TurfWall.h"

std::shared_ptr<SceneNode> buildHole14Node(CourseLayout& layout)
{
    auto node = std::make_shared<SceneNode>();
    constexpr int  IDX = 13;
    constexpr float fx = -2.857f, fz = 22.926f;

    node->addChild(fnNode([&layout](){ layout.drawGreenbed(IDX); }));
    node->addChild(fnNode([&layout](){ layout.drawGreen   (IDX); }));
    node->addChild(fnNode([&layout](){ layout.drawFlag    (IDX); }));
    node->addChild(shapeNode(TurfWall::createHole14()));
    node->addChild(makeFlagpole(fx, fz));
    node->addChild(makeHoleCup(fx, fz));
    node->addChild(makeBoulderCluster(fx - 1.5f, fz + 0.5f, 1.0f, 5, true));
    node->addChild(makeReedBed(fx + 1.0f, fz + 1.5f, 5, 0.4f));
    node->addChild(makeLampPost(fx + 2.5f, fz - 0.5f));
    return node;
}
