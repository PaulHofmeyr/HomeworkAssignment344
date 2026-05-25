#ifndef ROADNODE_H
#define ROADNODE_H
// ============================================================
//  RoadNode.h
//  Grey walkway from Path_coords.csv (regenerate via convert_path_coords.py).
// ============================================================
#include <memory>
#include "SceneNode.h"
#include "CourseLayout.h"

std::shared_ptr<SceneNode> buildRoadNode(CourseLayout& layout);
#endif
