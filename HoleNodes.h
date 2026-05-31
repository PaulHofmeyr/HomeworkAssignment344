#ifndef HOLENODES_H
#define HOLENODES_H

// ============================================================
//  HoleNodes.h
//
//  Each buildHoleXXNode() returns a complete SceneNode that
//  owns everything belonging to that hole:
//
//    ├── greenbedLayer   (sand + ring + centre, one BatchedFlat draw)
//    ├── greenLayer      (putting surface, one BatchedFlat draw)
//    ├── flagDiscLayer   (flat flag disc, one BatchedFlat draw)
//    ├── flagpole        (ProtoRegistry clone)
//    ├── holeCup         (ProtoRegistry clone)
//    └── obstacles       (boulders, shrubs, lamps, trees, …)
//
//  The layout reference must remain valid for the lifetime of
//  the scene (it is owned by SceneRoot).
// ============================================================

#include <memory>
#include "SceneNode.h"
#include "CourseLayout.h"

std::shared_ptr<SceneNode> buildHole01Node(CourseLayout& layout);
std::shared_ptr<SceneNode> buildHole02Node(CourseLayout& layout);
std::shared_ptr<SceneNode> buildHole03Node(CourseLayout& layout);
std::shared_ptr<SceneNode> buildHole04Node(CourseLayout& layout);
std::shared_ptr<SceneNode> buildHole05Node(CourseLayout& layout);
std::shared_ptr<SceneNode> buildHole06Node(CourseLayout& layout);
std::shared_ptr<SceneNode> buildHole07Node(CourseLayout& layout);
std::shared_ptr<SceneNode> buildHole08Node(CourseLayout& layout);
std::shared_ptr<SceneNode> buildHole09Node(CourseLayout& layout);
std::shared_ptr<SceneNode> buildHole10Node(CourseLayout& layout);
std::shared_ptr<SceneNode> buildHole11Node(CourseLayout& layout);
std::shared_ptr<SceneNode> buildHole12Node(CourseLayout& layout);
std::shared_ptr<SceneNode> buildHole13Node(CourseLayout& layout);
std::shared_ptr<SceneNode> buildHole14Node(CourseLayout& layout);
std::shared_ptr<SceneNode> buildHole15Node(CourseLayout& layout);
std::shared_ptr<SceneNode> buildHole16Node(CourseLayout& layout);
std::shared_ptr<SceneNode> buildHole17Node(CourseLayout& layout);
std::shared_ptr<SceneNode> buildHole18Node(CourseLayout& layout);

#endif // HOLENODES_H
