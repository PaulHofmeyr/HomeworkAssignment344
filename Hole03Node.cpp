#include "HoleNodes.h"
#include "NodeUtils.h"
#include "Prototype.h"
#include "CourseObjects.h"
#include "TurfWall.h"

std::shared_ptr<SceneNode> buildHole03Node(CourseLayout& layout)
{
    auto node = std::make_shared<SceneNode>();
    constexpr int  IDX = 2;
    constexpr float fx = 18.535f, fz = -17.767f;

    node->addChild(fnNode([&layout](){ layout.drawGreenbed(IDX); }));
    node->addChild(fnNode([&layout](){ layout.drawGreen   (IDX); }));
    node->addChild(fnNode([&layout](){ layout.drawFlag    (IDX); }));
    node->addChild(shapeNode(TurfWall::createHole03()));

    node->addChild(makeFlagpole(fx, fz));
    node->addChild(makeHoleCup(fx, fz));
    node->addChild(makeBoulderCluster(fx - 0.8f, fz + 0.8f, 0.8f, 5, true));
    node->addChild(makeShrubBed(fx + 1.5f, fz, 2, 0.4f));
    return node;
}
