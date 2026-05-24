#include "SceneRoot.h"
#include "NodeUtils.h"
#include "Prototype.h"
#include "CourseLayout.h"
#include "FloorNode.h"
#include "RoadNode.h"
#include "WaterNode.h"
#include "RocksNode.h"
#include "BridgeNode.h"
#include "HutNode.h"
#include "HoleNodes.h"
#include "WindmillNode.h"
#include "TreeNode.h"
#include "CourseObjects.h"

// ── Globals ──────────────────────────────────────────────────
std::shared_ptr<SceneNode> g_root;
SceneNode*                 g_rotorNode = nullptr;
std::vector<Shape*>        g_allShapes;
static CourseLayout*       g_layout = nullptr;

// ============================================================
void buildSceneRoot()
{
    // 1. Build all prototype master shapes FIRST
    //    (must happen after GL context is ready)
    ProtoRegistry::get().build();

    // 2. Build flat map geometry
    g_layout = new CourseLayout();
    g_layout->build();

    // 3. Root node
    g_root = std::make_shared<SceneNode>();

    // 4. Flat map layers
    g_root->addChild( buildFloorNode (*g_layout) );
    g_root->addChild( buildRoadNode  (*g_layout) );
    g_root->addChild( buildWaterNode (*g_layout) );
    g_root->addChild( buildRocksNode (*g_layout) );
    g_root->addChild( buildBridgeNode(*g_layout) );
    g_root->addChild( buildHutNode   (*g_layout) );

    // 5. All 18 holes — each node owns its flat layers + 3-D objects
    g_root->addChild( buildHole01Node(*g_layout) );
    g_root->addChild( buildHole02Node(*g_layout) );
    g_root->addChild( buildHole03Node(*g_layout) );
    g_root->addChild( buildHole04Node(*g_layout) );
    g_root->addChild( buildHole05Node(*g_layout) );
    g_root->addChild( buildHole06Node(*g_layout) );
    g_root->addChild( buildHole07Node(*g_layout) );
    g_root->addChild( buildHole08Node(*g_layout) );
    g_root->addChild( buildHole09Node(*g_layout) );
    g_root->addChild( buildHole10Node(*g_layout) );
    g_root->addChild( buildHole11Node(*g_layout) );
    g_root->addChild( buildHole12Node(*g_layout) );
    g_root->addChild( buildHole13Node(*g_layout) );
    g_root->addChild( buildHole14Node(*g_layout) );
    g_root->addChild( buildHole15Node(*g_layout) );
    g_root->addChild( buildHole16Node(*g_layout) );
    g_root->addChild( buildHole17Node(*g_layout) );
    g_root->addChild( buildHole18Node(*g_layout) );

    // 6. Windmill at hole 18
    auto [wmNode, rotorPtr] = buildWindmillNode();
    g_rotorNode = rotorPtr;
    g_root->addChild(wmNode);

    // 7. Perimeter trees
    g_root->addChild( buildTreeGroup() );

    // 8. Perimeter fence along course boundary (example)
    //    Add more sections here to fence the whole course.
    g_root->addChild(makeFenceSection(-20.f, -27.5f,  20.f, -27.5f, 8)); // south edge
    g_root->addChild(makeFenceSection( 20.f, -27.5f,  20.f,  27.5f, 8)); // east edge
    g_root->addChild(makeFenceSection( 20.f,  27.5f, -20.f,  27.5f, 8)); // north edge
    g_root->addChild(makeFenceSection(-20.f,  27.5f, -20.f, -27.5f, 8)); // west edge

    // 9. Lamp posts along main path (example positions)
    g_root->addChild(makeLampPost( 0.f, -25.f));
    g_root->addChild(makeLampPost( 8.f, -25.f));
    g_root->addChild(makeLampPost(-8.f, -25.f));

    // 10. Reed beds at pond edges
    g_root->addChild(makeReedBed( 5.f,  10.f, 8, 0.6f));
    g_root->addChild(makeReedBed(-3.f,  12.f, 6, 0.5f));
    g_root->addChild(makeReedBed( 8.f,   5.f, 7, 0.5f));
}

// ============================================================
void drawSceneRoot(GLuint shaderID, bool wireframe)
{
    if(g_root) g_root->draw(shaderID, wireframe);
}

// ============================================================
void updateRotor(const Matrix<4,4>& spinTransform)
{
    if(g_rotorNode)
        g_rotorNode->localTransform = spinTransform;
}

// ============================================================
void cleanupSceneRoot()
{
    g_root.reset();
    g_rotorNode = nullptr;

    for(auto* s : g_allShapes) delete s;
    g_allShapes.clear();

    ProtoRegistry::get().cleanup();

    if(g_layout){ delete g_layout; g_layout = nullptr; }
}
