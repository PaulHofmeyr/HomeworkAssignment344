#include "HoleNodes.h"
#include "NodeUtils.h"
#include "Prototype.h"
#include "CourseObjects.h"

std::shared_ptr<SceneNode> buildHole08Node()
{
    auto node = std::make_shared<SceneNode>();
    constexpr float fx = -0.855f, fz = -14.952f;
    node->addChild(makeFlagpole(fx, fz));
    node->addChild(makeHoleCup(fx, fz));
    node->addChild(makeBoulderCluster(fx - 1.0f, fz + 0.8f, 0.8f, 5));
    node->addChild(makeLampPost(fx + 1.5f, fz - 1.0f));
    node->addChild(makeShrubBed(fx + 0.5f, fz + 1.5f, 2, 0.5f));
    return node;
}
