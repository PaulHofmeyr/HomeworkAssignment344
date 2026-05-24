#ifndef BRIDGENODE_H
#define BRIDGENODE_H
// ============================================================
//  BridgeNode.h
//  The two wooden bridges over the water.
//  Owner: add bridge railings (Cuboid children), change colour here.
// ============================================================
#include <memory>
#include "SceneNode.h"
#include "CourseLayout.h"

std::shared_ptr<SceneNode> buildBridgeNode(CourseLayout& layout);
#endif
