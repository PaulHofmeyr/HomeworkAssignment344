#ifndef HOLENODE_H
#define HOLENODE_H
// ============================================================
//  HoleNode.h
//  One complete golf hole: sand greenbed rings, putting green,
//  3-D flag pole and hole cup.
//
//  Owner: add obstacles (ramps, walls, windmills) as extra
//  children to the returned node for that specific hole.
//
//  Usage:
//      auto h3 = buildHoleNode(layout, 2);  // hole 03 (0-indexed)
//      h3->addChild( myObstacleNode );       // attach obstacle
// ============================================================
#include <memory>
#include "SceneNode.h"
#include "CourseLayout.h"

// holeIndex = 0..17
std::shared_ptr<SceneNode> buildHoleNode(CourseLayout& layout, int holeIndex);
#endif
