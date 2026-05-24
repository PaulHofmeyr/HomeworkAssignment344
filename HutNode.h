#ifndef HUTNODE_H
#define HUTNODE_H
// ============================================================
//  HutNode.h
//  The covered gazebo / hut at the centre of the course.
//  Owner: modify roof shape, post positions, colours here.
// ============================================================
#include <memory>
#include "SceneNode.h"
#include "CourseLayout.h"

std::shared_ptr<SceneNode> buildHutNode(CourseLayout& layout);
#endif
