#ifndef WATERNODE_H
#define WATERNODE_H
// ============================================================
//  WaterNode.h
//  The three dams / water bodies on the course.
//  Owner: add ripple animation, reflection, colour changes here.
// ============================================================
#include <memory>
#include "SceneNode.h"
#include "CourseLayout.h"

std::shared_ptr<SceneNode> buildWaterNode(CourseLayout& layout);
#endif
