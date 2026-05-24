#include "CourseNode.h"
#include "Cuboid.h"
#include "Cylinder.h"
#include "Cone.h"
#include "TriangularPrism.h"
#include "Transformations.h"
#include <cmath>
#include <memory>

// ============================================================
//  Colour helpers (match Scene.cpp + CourseLayout.cpp palettes)
// ============================================================
// Course layout colours (same as CourseLayout.cpp)
#define COL_SAND    0.93f, 0.85f, 0.72f
#define COL_ROAD    0.58f, 0.58f, 0.58f
#define COL_WATER   0.15f, 0.55f, 0.85f
#define COL_ROCK    0.32f, 0.30f, 0.28f
#define COL_GBSAND  0.93f, 0.85f, 0.72f
#define COL_GBRING  0.55f, 0.80f, 0.40f
#define COL_GBCEN   0.20f, 0.65f, 0.18f
#define COL_GREEN   0.18f, 0.68f, 0.18f
#define COL_BRIDGE  0.55f, 0.42f, 0.25f
#define COL_HUT     0.78f, 0.78f, 0.78f
#define COL_FLAG    0.85f, 0.08f, 0.08f

// 3-D object colours
#define COL_STONE   0.78f, 0.70f, 0.55f
#define COL_DARK_CAP 0.22f, 0.22f, 0.22f
#define COL_ROOF    0.15f, 0.15f, 0.15f
#define COL_AXLE    0.60f, 0.60f, 0.65f
#define COL_BLADE   0.92f, 0.92f, 0.88f
#define COL_FLAGPOLE 0.80f, 0.80f, 0.80f
#define COL_FLAGBANNER 0.85f, 0.08f, 0.08f
#define COL_TRUNK   0.45f, 0.28f, 0.12f
#define COL_LEAVES  0.18f, 0.52f, 0.18f
#define COL_TRUNK2  0.30f, 0.18f, 0.08f
#define COL_LEAVES2 0.12f, 0.38f, 0.12f

// Y levels for flat layers (match CourseLayout)
static const float Y_FLOOR   = 0.000f;
static const float Y_ROAD    = 0.001f;
static const float Y_WATER   = 0.002f;
static const float Y_ROCK    = 0.003f;
static const float Y_GBSAND  = 0.004f;
static const float Y_GBRING  = 0.005f;
static const float Y_GBCEN   = 0.006f;
static const float Y_GREEN   = 0.007f;
static const float Y_BRIDGE  = 0.008f;
static const float Y_HUT     = 0.008f;
static const float Y_FLAG    = 0.009f;

// Map extents
static const float MAP_X_MIN = -20.0f;
static const float MAP_X_MAX =  20.0f;
static const float MAP_Z_MIN = -27.5f;
static const float MAP_Z_MAX =  27.5f;

// ============================================================
//  Exported globals
// ============================================================
std::shared_ptr<SceneNode> g_root;
SceneNode* g_rotorNode = nullptr;

// ============================================================
//  All heap-allocated geometry — kept so we can delete later
// ============================================================
static std::vector<Shape*> g_allShapes;

template<typename T, typename... Args>
T* makeShape(Args&&... args)
{
    T* s = new T(std::forward<Args>(args)...);
    g_allShapes.push_back(s);
    return s;
}

// ============================================================
//  Convenience: make a SceneNode leaf owning a shape
// ============================================================
static std::shared_ptr<SceneNode> leafNode(Shape* s)
{
    return std::make_shared<SceneNode>(s);
}

// ============================================================
//  shrinkPoly helper (same as CourseLayout)
// ============================================================
#include <algorithm>
static std::vector<std::pair<float,float>>
shrinkPoly(const float pts[][2], int n, float amount)
{
    float cx=0,cz=0;
    for(int i=0;i<n;++i){cx+=pts[i][0];cz+=pts[i][1];}
    cx/=n; cz/=n;
    std::vector<std::pair<float,float>> out(n);
    for(int i=0;i<n;++i){
        float dx=pts[i][0]-cx, dz=pts[i][1]-cz;
        float d=std::sqrt(dx*dx+dz*dz);
        if(d<1e-6f){out[i]={cx,cz};continue;}
        float nd=std::max(0.f,d-amount);
        out[i]={cx+dx/d*nd, cz+dz/d*nd};
    }
    return out;
}

// Build a FlatPolyShape from shrunk version of an outline
static FlatPolyShape* buildShrunkShape(const float pts[][2], int n,
                                        float shrink,
                                        float r, float g, float b, float y)
{
    auto inner = shrinkPoly(pts, n, shrink);
    std::vector<float> flat; flat.reserve(inner.size()*2);
    for(auto& p:inner){flat.push_back(p.first);flat.push_back(p.second);}
    const float(*fpts)[2] = reinterpret_cast<const float(*)[2]>(flat.data());
    auto* s = new FlatPolyShape();
    s->buildFrom(fpts,(int)inner.size(),r,g,b,y);
    g_allShapes.push_back(s);
    return s;
}

// Build a FlatPolyShape directly
static FlatPolyShape* buildPolyShape(const float pts[][2], int n,
                                      float r, float g, float b, float y)
{
    auto* s = new FlatPolyShape();
    s->buildFrom(pts,n,r,g,b,y);
    g_allShapes.push_back(s);
    return s;
}

// Build a rect FlatPolyShape
static FlatPolyShape* buildRectShape(float xMin,float xMax,float zMin,float zMax,
                                      float r,float g,float b,float y)
{
    float q[4][2]={{xMin,zMin},{xMax,zMin},{xMax,zMax},{xMin,zMax}};
    return buildPolyShape(q,4,r,g,b,y);
}

// Build a DiscShape
static DiscShape* buildDiscShape(float cx,float cz,float radius,
                                  float r,float g,float b,float y,int segs=14)
{
    auto* s = new DiscShape();
    s->buildFrom(cx,cz,radius,r,g,b,y,segs);
    g_allShapes.push_back(s);
    return s;
}

// ============================================================
//  All outline data – pulled from CourseLayout.cpp
//  We re-declare only the extern symbols we need.
//  Because CourseLayout.cpp defines them as file-static, we
//  instead include a minimal header that re-exposes them.
//  FASTER approach: just re-call buildPolyShape with the same
//  static arrays. We do this by including CourseLayout.cpp's
//  data section via a separate header.
//
//  Since the coordinate arrays in CourseLayout.cpp are file-
//  static, the cleanest zero-refactor solution is to let
//  CourseLayout own the GPU buffers AND wrap each one's
//  draw() through a thin SceneNode adapter (no Shape).
//  We use a "DrawableNode" that holds a raw draw-function.
// ============================================================

// ============================================================
//  DrawableNode – a SceneNode whose "shape" is a std::function
//  This lets us wrap FlatPoly/Disc draw() calls without
//  inheriting Shape, so CourseLayout's existing GPU buffers
//  are reused.
// ============================================================
#include <functional>

class DrawableFn : public Shape
{
    std::function<void()> m_fn;
public:
    DrawableFn(std::function<void()> fn) : Shape(1,1,1), m_fn(fn) {}
    void build() override {}
    void drawFilled()    const override { m_fn(); }
    void drawWireframe() const override { m_fn(); }
};

static DrawableFn* makeFn(std::function<void()> fn)
{
    auto* s = new DrawableFn(fn);
    g_allShapes.push_back(s);
    return s;
}

// ============================================================
//  Hole flag positions (from CourseLayout.cpp)
// ============================================================
static const float kHoleFlags[18][2] = {
    { 2.515f, -21.754f},  // 01
    {15.116f, -20.640f},  // 02
    {18.535f, -17.767f},  // 03
    {11.648f,  -6.684f},  // 04
    {17.118f,   4.984f},  // 05
    {11.795f,  14.483f},  // 06
    { 8.034f,  -2.580f},  // 07
    {-0.855f, -14.952f},  // 08
    {-6.227f, -19.584f},  // 09
    {-9.890f,  -1.114f},  // 10
    {-2.759f,  11.023f},  // 11
    { 2.076f,  19.643f},  // 12
    { 5.086f,  15.194f},  // 13
    {-2.857f,  22.926f},  // 14
    {-18.242f, 15.480f},  // 15
    {-12.772f, 11.375f},  // 16
    {-17.753f,  0.997f},  // 17
    {-16.337f, -14.542f}  // 18
};

// ============================================================
//  Build a 3-D flag pole at a given XZ position
//  (pole = slim Cylinder, banner = small Cuboid at top)
// ============================================================
static std::shared_ptr<SceneNode> buildFlagPole(float x, float z)
{
    auto group = std::make_shared<SceneNode>();
    // Pole: thin cylinder, 1.2 units tall
    auto* pole = makeShape<Cylinder>(x, 0.6f, z, 0.03f, 1.2f, 8, COL_FLAGPOLE);
    pole->build();
    group->addChild(leafNode(pole));
    // Banner: small cuboid at top
    auto* banner = makeShape<Cuboid>(x+0.12f, 1.1f, z, 0.24f, 0.15f, 0.04f, COL_FLAGBANNER);
    banner->build();
    group->addChild(leafNode(banner));
    return group;
}

// ============================================================
//  Build the windmill node tree
//  Windmill is at hole 18 position: (-16.337, 0, -14.542)
//  but lifted to sit on the ground (y=0 base)
// ============================================================
static std::shared_ptr<SceneNode> buildWindmill()
{
    auto wmNode = std::make_shared<SceneNode>();
    // Place at hole 18
    wmNode->localTransform = makeTranslation3D(-16.337f, 0.0f, -14.542f);

    // Scale up slightly so it's visible at course scale
    // (course is ~40x55 world units, windmill from prac3 was ~1x1 scale)
    // We scale the group 1.5x so it reads well
    Matrix<4,4> scl = makeScale3D(1.5f, 1.5f, 1.5f);
    wmNode->localTransform = wmNode->localTransform * scl;

    // Legs (6 stone cuboids around base)
    float legR=0.38f, legHW=0.07f, legHH=0.28f, legHD=0.07f, legCY=legHH;
    float legAngles[6] = {0.f, 60.f, 120.f, 180.f, 240.f, 300.f};
    for(int i=0;i<6;++i){
        float a = legAngles[i] * 3.14159f / 180.f;
        auto* leg = makeShape<Cuboid>(legR*std::cos(a), legCY, legR*std::sin(a),
                                      legHW, legHH, legHD, COL_STONE);
        leg->build();
        wmNode->addChild(leafNode(leg));
    }

    // Tower
    auto* tower = makeShape<Cylinder>(0.f, 0.84f, 0.f, 0.35f, 0.95f, 16, COL_STONE);
    tower->build();
    wmNode->addChild(leafNode(tower));

    // Dark cap
    auto* cap = makeShape<Cylinder>(0.f, 1.40f, 0.f, 0.37f, 0.18f, 16, COL_DARK_CAP);
    cap->build();
    wmNode->addChild(leafNode(cap));

    // Cone roof
    auto* roof = makeShape<Cone>(0.f, 1.49f, 0.f, 0.37f, 0.40f, 16, COL_ROOF);
    roof->build();
    wmNode->addChild(leafNode(roof));

    // Rotor group – main.cpp updates this node's localTransform each frame
    auto rotorNode = std::make_shared<SceneNode>(); // identity initially
    g_rotorNode = rotorNode.get();

    auto* axle = makeShape<Cylinder>(0.f, 1.48f, 0.43f, 0.06f, 0.03f, 12, COL_AXLE);
    axle->build();
    rotorNode->addChild(leafNode(axle));

    float bCX=0.f, bCY=1.48f, bCZ=0.43f, bLen=0.35f, bW=0.05f, bD=0.015f;
    auto* b0 = makeShape<Cuboid>(bCX,        bCY+bLen/2.f, bCZ, bW,   bLen, bD, COL_BLADE);
    auto* b1 = makeShape<Cuboid>(bCX+bLen/2.f, bCY,        bCZ, bLen, bW,   bD, COL_BLADE);
    auto* b2 = makeShape<Cuboid>(bCX,        bCY-bLen/2.f, bCZ, bW,   bLen, bD, COL_BLADE);
    auto* b3 = makeShape<Cuboid>(bCX-bLen/2.f, bCY,        bCZ, bLen, bW,   bD, COL_BLADE);
    b0->build(); b1->build(); b2->build(); b3->build();
    rotorNode->addChild(leafNode(b0));
    rotorNode->addChild(leafNode(b1));
    rotorNode->addChild(leafNode(b2));
    rotorNode->addChild(leafNode(b3));

    wmNode->addChild(rotorNode);
    return wmNode;
}

// ============================================================
//  Build decorative trees scattered around course boundary
// ============================================================
static std::shared_ptr<SceneNode> buildTreeGroup()
{
    auto group = std::make_shared<SceneNode>();

    // Round trees (cone-top) near course perimeter
    float rx[6] = {-19.f,  19.f, -19.f,  19.f, -10.f,  10.f};
    float rz[6] = {-25.f, -20.f,  20.f,  20.f,  26.f,  26.f};
    for(int i=0;i<6;++i){
        auto treeNode = std::make_shared<SceneNode>();
        auto* trunk = makeShape<Cylinder>(rx[i], 0.4f, rz[i], 0.15f, 0.8f, 8, COL_TRUNK);
        auto* top   = makeShape<Cone>    (rx[i], 1.2f, rz[i], 0.55f, 1.1f, 8, COL_LEAVES);
        trunk->build(); top->build();
        treeNode->addChild(leafNode(trunk));
        treeNode->addChild(leafNode(top));
        group->addChild(treeNode);
    }

    // Prism-top trees
    float px[4] = {-19.f,  19.f, -15.f,  15.f};
    float pz[4] = {   5.f,  -5.f,  26.f,  26.f};
    for(int i=0;i<4;++i){
        auto treeNode = std::make_shared<SceneNode>();
        auto* trunk = makeShape<Cylinder>   (px[i], 0.4f, pz[i], 0.15f, 0.8f, 8, COL_TRUNK2);
        auto* top   = makeShape<TriangularPrism>(px[i], 1.2f, pz[i], 0.55f, 0.55f, 0.55f, COL_LEAVES2);
        trunk->build(); top->build();
        treeNode->addChild(leafNode(trunk));
        treeNode->addChild(leafNode(top));
        group->addChild(treeNode);
    }

    return group;
}

// ============================================================
//  Build Gazebo node (simple version – hexagonal roof + 6 posts)
//  Positioned at centre of course (approx 0, 0)
// ============================================================
static std::shared_ptr<SceneNode> buildGazebo()
{
    auto group = std::make_shared<SceneNode>();
    group->localTransform = makeTranslation3D(0.f, 0.f, 0.f);

    // 6 posts in a hex ring, radius 2.5 units, height 2.0
    for(int i=0;i<6;++i){
        float a = i * 3.14159f / 3.f;
        float px = 2.5f * std::cos(a);
        float pz = 2.5f * std::sin(a);
        auto* post = makeShape<Cylinder>(px, 1.0f, pz, 0.12f, 2.0f, 8,
                                          0.55f, 0.38f, 0.18f); // timber
        post->build();
        group->addChild(leafNode(post));
    }

    // Hexagonal flat deck (just a cylinder approximating hex)
    auto* deck = makeShape<Cylinder>(0.f, 0.05f, 0.f, 2.6f, 0.08f, 6,
                                      0.65f, 0.48f, 0.25f); // timber deck
    deck->build();
    group->addChild(leafNode(deck));

    // Roof – cone on top
    auto* roof = makeShape<Cone>(0.f, 2.1f, 0.f, 2.8f, 1.4f, 6,
                                  0.20f, 0.20f, 0.20f); // dark metal
    roof->build();
    group->addChild(leafNode(roof));

    return group;
}

// ============================================================
//  buildCourseGraph  – the main entry point
// ============================================================

// Forward-declare the CourseLayout instance we'll use for draw() callbacks
static CourseLayout* g_layout = nullptr;

void buildCourseGraph()
{
    // Build the flat 2-D map geometry (existing system)
    g_layout = new CourseLayout();
    g_layout->build();

    // ── Root ───────────────────────────────────────────────
    g_root = std::make_shared<SceneNode>();
    // Slight upward tilt so the flat map faces the camera better in
    // default perspective view (rotate -15° around X so it's not
    // completely bird-eye flat)
    // Comment this out if you prefer top-down ortho
    // g_root->localTransform = makeRotationX(-0.26f); // ~-15 deg

    // ── Course node (contains all 2-D flat map objects) ────
    auto courseNode = std::make_shared<SceneNode>();
    // No transform – map is already in world space

    // Floor
    courseNode->addChild(leafNode(makeFn([](){ /* drawn via layout */ })));
    // We'll draw the full layout via a single callback node for simplicity:
    // One "layout node" wraps the whole CourseLayout draw() call
    auto layoutNode = std::make_shared<SceneNode>(
        makeFn([](){ g_layout->draw(); })
    );
    courseNode->addChild(layoutNode);

    // ── Per-hole 3-D flag poles ─────────────────────────────
    // Each hole gets its own group node with a flag pole child.
    // This is where you'd later add walls, ramps, obstacles per hole.
    auto holesGroup = std::make_shared<SceneNode>();
    for(int i=0;i<18;++i){
        auto holeNode = std::make_shared<SceneNode>();
        // Flag pole
        holeNode->addChild(buildFlagPole(kHoleFlags[i][0], kHoleFlags[i][1]));

        // Hole cup (small dark cylinder sunk into green)
        auto* cup = makeShape<Cylinder>(kHoleFlags[i][0], 0.01f, kHoleFlags[i][1],
                                         0.1f, 0.05f, 10, 0.05f, 0.05f, 0.05f);
        cup->build();
        holeNode->addChild(leafNode(cup));

        holesGroup->addChild(holeNode);
    }
    courseNode->addChild(holesGroup);

    g_root->addChild(courseNode);

    // ── Gazebo at centre ────────────────────────────────────
    g_root->addChild(buildGazebo());

    // ── Windmill at hole 18 ─────────────────────────────────
    g_root->addChild(buildWindmill());

    // ── Trees around perimeter ─────────────────────────────
    g_root->addChild(buildTreeGroup());
}

// ============================================================
//  drawCourseGraph
// ============================================================
void drawCourseGraph(GLuint shaderID, bool wireframe)
{
    if(!g_root) return;
    g_root->draw(shaderID, wireframe);
}

// ============================================================
//  updateRotor  – call each frame with the accumulated spin matrix
// ============================================================
void updateRotor(const Matrix<4,4>& spinTransform)
{
    if(g_rotorNode)
        g_rotorNode->localTransform = spinTransform;
}

// ============================================================
//  cleanupCourseGraph
// ============================================================
void cleanupCourseGraph()
{
    g_root.reset();
    g_rotorNode = nullptr;

    for(auto* s : g_allShapes) delete s;
    g_allShapes.clear();

    if(g_layout){ delete g_layout; g_layout = nullptr; }
}
