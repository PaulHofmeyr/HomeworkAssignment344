#ifndef ROCKSNODE_H
#define ROCKSNODE_H
// ============================================================
//  RocksNode.h
//  Rock beds (filled polys) and individual rock discs along
//  the water edges.
//  Owner: adjust rock colours / sizes / positions here.
// ============================================================
#include <memory>
#include "SceneNode.h"
#include "CourseLayout.h"

std::shared_ptr<SceneNode> buildRocksNode(CourseLayout& layout);
#endif
