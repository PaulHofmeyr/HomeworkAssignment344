#ifndef ROCKSNODE_H
#define ROCKSNODE_H
// ============================================================
//  RocksNode.h
//  Rock beds: 3D boulders at each map rock-bed disc position.
//  Owner: adjust rock colours / sizes / positions here.
// ============================================================
#include <memory>
#include "SceneNode.h"
#include "CourseLayout.h"

std::shared_ptr<SceneNode> buildRocksNode(CourseLayout& layout);
#endif
