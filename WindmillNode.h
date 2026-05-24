#ifndef WINDMILLNODE_H
#define WINDMILLNODE_H
// ============================================================
//  WindmillNode.h
//  The 3-D windmill obstacle sitting at hole 18.
//  Owner: change blade count / size / tower height here.
//
//  After building, call:
//      windmillNode->g_rotorNode->localTransform = spinMatrix;
//  each frame to animate the rotor.
// ============================================================
#include <memory>
#include "SceneNode.h"

struct WindmillResult {
    std::shared_ptr<SceneNode> root;   // place this in the scene graph
    SceneNode* rotorNode;              // update localTransform each frame
};

WindmillResult buildWindmillNode();
#endif
