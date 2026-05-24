#include "HoleNodes.h"
#include "NodeUtils.h"
#include "Prototype.h"
#include "CourseObjects.h"

std::shared_ptr<SceneNode> buildHole13Node(CourseLayout& layout)
{
    auto node = std::make_shared<SceneNode>();
    constexpr int  IDX = 12;
    constexpr float fx =  5.086f, fz = 15.194f;

    node->addChild(fnNode([&layout](){ layout.drawGreenbed(IDX); }));
    node->addChild(fnNode([&layout](){ layout.drawGreen   (IDX); }));
    node->addChild(fnNode([&layout](){ layout.drawFlag    (IDX); }));

    node->addChild(makeFlagpole(fx, fz));
    node->addChild(makeHoleCup(fx, fz));
    node->addChild(makeBoulderCluster(fx + 1.2f, fz + 1.0f, 0.7f, 5));
    node->addChild(makeGrassClump(fx - 1.0f, fz, 4, 0.4f));
    return node;
}
