#include "HoleNodes.h"
#include "NodeUtils.h"
#include "Prototype.h"
#include "CourseObjects.h"

std::shared_ptr<SceneNode> buildHole07Node()
{
    auto node = std::make_shared<SceneNode>();
    constexpr float fx = 8.034f, fz = -2.580f;
    node->addChild(makeFlagpole(fx, fz));
    node->addChild(makeHoleCup(fx, fz));
    node->addChild(makeBoulderCluster(fx, fz + 1.2f, 0.6f, 3));
    node->addChild(makeGrassClump(fx - 1.2f, fz, 4, 0.4f));
    return node;
}
