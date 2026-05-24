#include "HoleNodes.h"
#include "NodeUtils.h"
#include "Prototype.h"
#include "CourseObjects.h"

std::shared_ptr<SceneNode> buildHole01Node()
{
    auto node = std::make_shared<SceneNode>();
    constexpr float fx =  2.515f, fz = -21.754f;
    node->addChild(makeFlagpole(fx, fz));
    node->addChild(makeHoleCup(fx, fz));
    node->addChild(makeBoulderCluster(fx - 1.2f, fz + 0.5f, 0.7f, 4));
    node->addChild(makeShrubBed(fx + 0.8f, fz + 1.0f, 3, 0.5f));
    node->addChild(makeLampPost(fx - 2.0f, fz - 1.5f));
    return node;
}
