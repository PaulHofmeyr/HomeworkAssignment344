#include "WaterNode.h"
#include "NodeUtils.h"

std::shared_ptr<SceneNode> buildWaterNode(CourseLayout& layout)
{
    return fnNode([&layout](){ layout.drawWater(); });
}
