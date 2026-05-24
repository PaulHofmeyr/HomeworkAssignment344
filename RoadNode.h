#ifndef ROADNODE_H
#define ROADNODE_H
// ============================================================
//  RoadNode.h
//  The grey concrete path that winds around the whole course.
//  Owner: change road colour or add road markings here.
// ============================================================
#include <memory>
#include "SceneNode.h"
#include "CourseLayout.h"

std::shared_ptr<SceneNode> buildRoadNode(CourseLayout& layout);
#endif
