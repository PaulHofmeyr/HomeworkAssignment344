#include "HoleNodes.h"
#include "NodeUtils.h"
#include "Prototype.h"
#include "CourseObjects.h"

std::shared_ptr<SceneNode> buildHole09Node()
{
    auto node = std::make_shared<SceneNode>();
    constexpr float fx = -6.227f, fz = -19.584f;
    node->addChild(makeFlagpole(fx, fz));
    node->addChild(makeHoleCup(fx, fz));
    node->addChild(makeBoulderCluster(fx + 0.8f, fz + 0.8f, 0.7f, 4, true));
    node->addChild(makeGrassClump(fx - 1.0f, fz - 0.5f, 5, 0.3f));
    return node;
}
