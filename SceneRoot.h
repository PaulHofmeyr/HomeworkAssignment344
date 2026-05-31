#ifndef SCENEROOT_H
#define SCENEROOT_H
// ============================================================
//  SceneRoot.h
//
//  Assembles every individual node into one scene graph.
//  This is the ONLY file main.cpp needs to include.
//
//  Scene graph structure
//  ─────────────────────
//  g_root
//  ├── floorNode          FloorNode.cpp
//  ├── roadNode           RoadNode.cpp
//  ├── waterNode          WaterNode.cpp
//  ├── rocksNode          RocksNode.cpp
//  ├── bridgeNode         BridgeNode.cpp
//  ├── hutNode            HutNode.cpp
//  ├── hole[0..17]Node    HoleNode.cpp  (each hole is its own node)
//  ├── windmillNode       WindmillNode.cpp
//  └── treeGroup          TreeNode.cpp
// ============================================================

#include <memory>
#include "SceneNode.h"
#include "Transformations.h"
#include <GL/glew.h>

extern std::shared_ptr<SceneNode> g_root;
extern SceneNode* g_rotorNode;   // set by buildSceneRoot(), used by main.cpp

void buildSceneRoot();
void drawSceneRoot(GLuint shaderID, bool wireframe);
void cleanupSceneRoot();
void updateRotor(const Matrix<4,4>& spinTransform);

#endif // SCENEROOT_H
