#include "SceneRoot.h"
#include "NodeUtils.h"
#include "CourseLayout.h"
#include "FloorNode.h"
#include "RoadNode.h"
#include "WaterNode.h"
#include "RocksNode.h"
#include "BridgeNode.h"
#include "HutNode.h"
#include "HoleNode.h"
#include "WindmillNode.h"
#include "TreeNode.h"

// ── Globals ──────────────────────────────────────────────────
std::shared_ptr<SceneNode> g_root;
SceneNode*                 g_rotorNode = nullptr;

// Shape registry (defined here, declared extern in NodeUtils.h)
std::vector<Shape*> g_allShapes;

// The CourseLayout instance (owns all flat-map GPU buffers)
static CourseLayout* g_layout = nullptr;

// ────────────────────────────────────────────────────────────
void buildSceneRoot()
{
    // 1. Build the flat map geometry
    g_layout = new CourseLayout();
    g_layout->build();

    // 2. Create root node (identity transform = world origin)
    g_root = std::make_shared<SceneNode>();

    // 3. Flat map layers ─────────────────────────────────────
    g_root->addChild( buildFloorNode (*g_layout) );
    g_root->addChild( buildRoadNode  (*g_layout) );
    g_root->addChild( buildWaterNode (*g_layout) );
    g_root->addChild( buildRocksNode (*g_layout) );
    g_root->addChild( buildBridgeNode(*g_layout) );
    g_root->addChild( buildHutNode   (*g_layout) );

    // 4. All 18 holes ────────────────────────────────────────
    //    Each hole is its own node — add obstacles as children here:
    //
    //    auto h = buildHoleNode(*g_layout, 17);  // hole 18
    //    h->addChild( buildWindmillNode().root ); // attach obstacle
    //    g_root->addChild(h);
    //
    for(int i = 0; i < 18; ++i)
        g_root->addChild( buildHoleNode(*g_layout, i) );

    // 5. Windmill at hole 18 ──────────────────────────────────
    auto [wmNode, rotorPtr] = buildWindmillNode();
    g_rotorNode = rotorPtr;
    g_root->addChild(wmNode);

    // 6. Perimeter trees ──────────────────────────────────────
    g_root->addChild( buildTreeGroup() );
}

// ────────────────────────────────────────────────────────────
void drawSceneRoot(GLuint shaderID, bool wireframe)
{
    if(g_root) g_root->draw(shaderID, wireframe);
}

// ────────────────────────────────────────────────────────────
void updateRotor(const Matrix<4,4>& spinTransform)
{
    if(g_rotorNode)
        g_rotorNode->localTransform = spinTransform;
}

// ────────────────────────────────────────────────────────────
void cleanupSceneRoot()
{
    g_root.reset();
    g_rotorNode = nullptr;

    for(auto* s : g_allShapes) delete s;
    g_allShapes.clear();

    if(g_layout){ delete g_layout; g_layout = nullptr; }
}
