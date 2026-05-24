#ifndef HOLENODES_H
#define HOLENODES_H

#include <memory>
#include "SceneNode.h"

// One builder per hole — each places the 3-D objects only
// (flagpole, hole cup, obstacles).  Flat green/greenbed/flag
// geometry is drawn as a single batched call from SceneRoot.

std::shared_ptr<SceneNode> buildHole01Node();
std::shared_ptr<SceneNode> buildHole02Node();
std::shared_ptr<SceneNode> buildHole03Node();
std::shared_ptr<SceneNode> buildHole04Node();
std::shared_ptr<SceneNode> buildHole05Node();
std::shared_ptr<SceneNode> buildHole06Node();
std::shared_ptr<SceneNode> buildHole07Node();
std::shared_ptr<SceneNode> buildHole08Node();
std::shared_ptr<SceneNode> buildHole09Node();
std::shared_ptr<SceneNode> buildHole10Node();
std::shared_ptr<SceneNode> buildHole11Node();
std::shared_ptr<SceneNode> buildHole12Node();
std::shared_ptr<SceneNode> buildHole13Node();
std::shared_ptr<SceneNode> buildHole14Node();
std::shared_ptr<SceneNode> buildHole15Node();
std::shared_ptr<SceneNode> buildHole16Node();
std::shared_ptr<SceneNode> buildHole17Node();
std::shared_ptr<SceneNode> buildHole18Node();

#endif // HOLENODES_H
