#include "HoleNodes.h"
#include "NodeUtils.h"
#include "Prototype.h"
#include "CourseObjects.h"
#include "TurfWall.h"

std::shared_ptr<SceneNode> buildHole17Node(CourseLayout& layout)
{
    auto node = std::make_shared<SceneNode>();
    constexpr int  IDX = 16;
    constexpr float fx = -17.753f, fz = 0.997f;

    node->addChild(fnNode([&layout](){ layout.drawGreenbed(IDX); }));
    node->addChild(fnNode([&layout](){ layout.drawGreen   (IDX); }));
    node->addChild(fnNode([&layout](){ layout.drawFlag    (IDX); }));
    node->addChild(shapeNode(TurfWall::createHole17()));
    node->addChild(makeFlagpole(fx, fz));
    node->addChild(makeHoleCup(fx, fz));
    node->addChild(makeBoulderCluster(fx - 1.0f, fz + 0.8f, 0.8f, 4, true));
    node->addChild(makeFenceSection(fx - 2.f, fz - 1.f, fx + 0.5f, fz - 1.f, 3));
    node->addChild(makeLampPost(fx + 1.5f, fz + 1.5f));
    return node;
}
