#include "HoleNodes.h"
#include "NodeUtils.h"
#include "Prototype.h"
#include "CourseObjects.h"

std::shared_ptr<SceneNode> buildHole11Node(CourseLayout& layout)
{
    auto node = std::make_shared<SceneNode>();
    constexpr int  IDX = 10;
    constexpr float fx = -2.759f, fz = 11.023f;

    node->addChild(fnNode([&layout](){ layout.drawGreenbed(IDX); }));
    node->addChild(fnNode([&layout](){ layout.drawGreen   (IDX); }));
    node->addChild(fnNode([&layout](){ layout.drawFlag    (IDX); }));

    node->addChild(makeFlagpole(fx, fz));
    node->addChild(makeHoleCup(fx, fz));
    node->addChild(makeBoulderCluster(fx + 1.0f, fz, 0.6f, 3));
    node->addChild(makeShrubBed(fx - 0.8f, fz + 1.2f, 4, 0.6f));
    node->addChild(makeLampPost(fx + 2.0f, fz + 1.5f));
    return node;
}
