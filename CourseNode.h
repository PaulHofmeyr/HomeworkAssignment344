#ifndef COURSENODE_H
#define COURSENODE_H

// ============================================================
//  CourseNode.h
//
//  Thin Shape wrappers so FlatPoly / Disc can live in SceneNodes,
//  plus factory functions that build the full scene graph for
//  the entire Crescent Head course.
//
//  Scene graph structure
//  ─────────────────────
//  g_root  (identity)
//  ├── courseNode          ← entire 2-D map (translate / scale here)
//  │   ├── floorNode       ← sand base
//  │   ├── roadNode        ← grey concrete paths
//  │   ├── waterNode       ← group
//  │   │   ├── mainDamNode
//  │   │   └── pond[0..3]
//  │   ├── rocksNode       ← group (water-edge + rock-bed discs)
//  │   ├── hole[0..17]Node ← per-hole group
//  │   │   ├── greenbedSandNode
//  │   │   ├── greenbedRingNode
//  │   │   ├── greenbedCentreNode
//  │   │   ├── greenNode
//  │   │   └── flagNode
//  │   ├── bridgeNode[0..1]
//  │   └── hutNode
//  ├── windmillNode        ← translated to hole-18 position
//  │   ├── baseLegs[0..5]
//  │   ├── towerNode
//  │   ├── roofNode
//  │   └── rotorNode       ← spin transform applied here every frame
//  │       ├── axleNode
//  │       └── blade[0..3]
//  └── treeGroup[0..7]     ← round + prism trees around the course
//      ├── trunkNode
//      └── topNode
// ============================================================

#include <GL/glew.h>
#include <vector>
#include <memory>
#include "Shape.h"
#include "SceneNode.h"
#include "CourseLayout.h"

// ============================================================
//  FlatPolyShape  – wraps a FlatPoly as a Shape so SceneNode
//  can draw it through the standard drawFilled()/drawWireframe()
//  interface.  Wireframe just re-issues the same draw (acceptable
//  for flat polygons; they show their edges under GL_LINE mode).
// ============================================================
class FlatPolyShape : public Shape
{
public:
    FlatPoly poly;

    FlatPolyShape() : Shape(1,1,1) {}

    // Build from an outline array
    void buildFrom(const float pts[][2], int n,
                   float r, float g, float b, float y = 0.0f)
    {
        poly.build(pts, n, r, g, b, y);
    }

    // Shape interface
    void build() override {} // no-op; geometry already on GPU

    void drawFilled()    const override { poly.draw(); }
    void drawWireframe() const override { poly.draw(); }
};

// ============================================================
//  DiscShape  – wraps a Disc as a Shape
// ============================================================
class DiscShape : public Shape
{
public:
    Disc disc;

    DiscShape() : Shape(1,1,1) {}

    void buildFrom(float cx, float cz, float radius,
                   float r, float g, float b,
                   float y = 0.0f, int segs = 14)
    {
        disc.build(cx, cz, radius, r, g, b, y, segs);
    }

    void build() override {}
    void drawFilled()    const override { disc.draw(); }
    void drawWireframe() const override { disc.draw(); }
};

// ============================================================
//  Exported pointers – set by buildCourseGraph()
// ============================================================

// Root of the entire scene
extern std::shared_ptr<SceneNode> g_root;

// Kept so main.cpp can update the spin each frame
extern SceneNode* g_rotorNode;

// ============================================================
//  Build / draw / cleanup
// ============================================================
void buildCourseGraph();   // call once after OpenGL is ready
void drawCourseGraph(GLuint shaderID, bool wireframe);
void cleanupCourseGraph();

// Called each frame by main to animate the rotor
void updateRotor(const Matrix<4,4>& spinTransform);

#endif // COURSENODE_H
