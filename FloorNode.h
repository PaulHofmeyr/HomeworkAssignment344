#ifndef FLOORNODE_H
#define FLOORNODE_H
// ============================================================
//  FloorNode.h
//  The sand base rectangle covering the whole course.
//  Owner: change floor colour / scale here.
// ============================================================
#include <memory>
#include "SceneNode.h"
#include "CourseLayout.h"

std::shared_ptr<SceneNode> buildFloorNode(CourseLayout& layout);
#endif
