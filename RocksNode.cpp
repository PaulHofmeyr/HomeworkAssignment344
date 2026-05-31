#include "RocksNode.h"
#include "CourseObjects.h"

std::shared_ptr<SceneNode> buildRocksNode(CourseLayout&)
{
    return makeRockBedBoulders(
        courseRockBedDiscCount(),
        courseRockBedDiscs());
}
