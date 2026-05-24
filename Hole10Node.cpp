#include "HoleNodes.h"
#include "NodeUtils.h"
#include "Prototype.h"
#include "CourseObjects.h"

std::shared_ptr<SceneNode> buildHole10Node()
{
    auto node = std::make_shared<SceneNode>();
    constexpr float fx = -9.890f, fz = -1.114f;
    node->addChild(makeFlagpole(fx, fz));
    node->addChild(makeHoleCup(fx, fz));
    node->addChild(makeBoulderCluster(fx - 1.2f, fz - 1.0f, 0.9f, 4));
    node->addChild(makeNativeTree(fx - 2.5f, fz, 0.8f));
    node->addChild(makeNativeTree(fx - 3.0f, fz + 1.0f));
    return node;
}
