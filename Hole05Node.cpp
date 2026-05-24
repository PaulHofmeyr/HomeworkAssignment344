#include "HoleNodes.h"
#include "NodeUtils.h"
#include "Prototype.h"
#include "CourseObjects.h"

std::shared_ptr<SceneNode> buildHole05Node()
{
    auto node = std::make_shared<SceneNode>();
    constexpr float fx = 17.118f, fz = 4.984f;
    node->addChild(makeFlagpole(fx, fz));
    node->addChild(makeHoleCup(fx, fz));
    node->addChild(makeBoulderCluster(fx - 1.0f, fz - 1.0f, 0.9f, 4, true));
    node->addChild(makeFenceSection(fx - 2.f, fz + 1.f, fx + 1.f, fz + 1.f, 4));
    return node;
}
