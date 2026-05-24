#include "HoleNodes.h"
#include "NodeUtils.h"
#include "Prototype.h"
#include "CourseObjects.h"

std::shared_ptr<SceneNode> buildHole04Node()
{
    auto node = std::make_shared<SceneNode>();
    constexpr float fx = 11.648f, fz = -6.684f;
    node->addChild(makeFlagpole(fx, fz));
    node->addChild(makeHoleCup(fx, fz));
    node->addChild(makeBoulderCluster(fx, fz - 1.0f, 0.6f, 3));
    node->addChild(makeGrassClump(fx + 1.0f, fz + 0.5f, 5, 0.3f));
    node->addChild(makeLampPost(fx + 2.5f, fz));
    return node;
}
