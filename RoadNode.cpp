#include "RoadNode.h"
#include "NodeUtils.h"

std::shared_ptr<SceneNode> buildRoadNode(CourseLayout& layout)
{
    return fnNode([&layout](){ layout.drawRoad(); });
}
