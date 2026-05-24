#include "RocksNode.h"
#include "NodeUtils.h"

std::shared_ptr<SceneNode> buildRocksNode(CourseLayout& layout)
{
    return fnNode([&layout](){ layout.drawRocks(); });
}
