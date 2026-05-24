#include "FloorNode.h"
#include "NodeUtils.h"

std::shared_ptr<SceneNode> buildFloorNode(CourseLayout& layout)
{
    return fnNode([&layout](){ layout.drawFloor(); });
}
