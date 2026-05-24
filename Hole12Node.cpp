#include "HoleNodes.h"
#include "NodeUtils.h"
#include "Prototype.h"
#include "CourseObjects.h"

std::shared_ptr<SceneNode> buildHole12Node(CourseLayout& layout)
{
    auto node = std::make_shared<SceneNode>();
    constexpr int  IDX = 11;
    constexpr float fx =  2.076f, fz = 19.643f;

    node->addChild(fnNode([&layout](){ layout.drawGreenbed(IDX); }));
    node->addChild(fnNode([&layout](){ layout.drawGreen   (IDX); }));
    node->addChild(fnNode([&layout](){ layout.drawFlag    (IDX); }));

    node->addChild(makeFlagpole(fx, fz));
    node->addChild(makeHoleCup(fx, fz));
    node->addChild(makeBoulderCluster(fx - 0.8f, fz - 0.8f, 0.8f, 4, true));
    node->addChild(makeFenceSection(fx + 1.5f, fz - 1.f, fx + 1.5f, fz + 1.5f, 3));
    return node;
}
