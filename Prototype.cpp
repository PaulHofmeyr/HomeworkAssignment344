#include "Prototype.h"
#include "Cylinder.h"
#include "Cuboid.h"
#include "Cone.h"
#include "TriangularPrism.h"
#include "NodeUtils.h"

// ============================================================
//  Colour palette  (RGB floats, matches course aesthetic)
// ============================================================
// Boulders
#define C_GRANITE_D  0.38f, 0.34f, 0.30f   // dark granite
#define C_GRANITE_L  0.55f, 0.52f, 0.48f   // light granite
#define C_SANDSTONE  0.72f, 0.58f, 0.38f   // warm sandstone
// Infrastructure
#define C_STEEL      0.60f, 0.62f, 0.65f   // lamp post / fence metal
#define C_LAMP_HEAD  0.90f, 0.88f, 0.70f   // warm lamp globe
#define C_TIMBER     0.55f, 0.38f, 0.18f   // fence rail wood (hole sections)
#define C_FENCE      0.05f, 0.05f, 0.05f   // perimeter fence posts
#define C_CONCRETE   0.68f, 0.68f, 0.65f   // path edge / kerb
// Course objects
#define C_FLAG_POLE  0.80f, 0.80f, 0.80f   // silver pole
#define C_FLAG_BAN   0.85f, 0.08f, 0.08f   // red banner
#define C_CUP        0.05f, 0.05f, 0.05f   // near-black cup
// Plants
#define C_SHRUB      0.22f, 0.52f, 0.18f   // dark shrub green
#define C_GRASS      0.38f, 0.62f, 0.22f   // ornamental grass
#define C_LEAF       0.18f, 0.52f, 0.18f   // tree canopy
#define C_TRUNK      0.45f, 0.28f, 0.12f   // tree trunk
#define C_REED       0.42f, 0.55f, 0.28f   // reed green

// ============================================================
//  build()  –  called once at startup
//
//  ALL MASTER SHAPES ARE BUILT AT ORIGIN (0,0,0).
//  place() / clone() apply transforms via localTransform.
//  So all positions/sizes here are RELATIVE (unit-scale).
// ============================================================
void ProtoRegistry::build()
{
    // ── BOULDER_SMALL  (~0.3m diam) ──────────────────────────
    //  A short wide cylinder with squashed height = natural
    //  boulder look.  Sides=10 gives a good irregular feel.
    {
        auto* s = new Cylinder(0.f, 0.10f, 0.f,
                               0.18f, 0.20f, 10,
                               C_GRANITE_D);
        s->build();
        m_masters[BOULDER_SMALL] = s;
    }

    // ── BOULDER_MED  (~0.7m diam) ────────────────────────────
    {
        auto* s = new Cylinder(0.f, 0.22f, 0.f,
                               0.38f, 0.44f, 10,
                               C_GRANITE_D);
        s->build();
        m_masters[BOULDER_MED] = s;
    }

    // ── BOULDER_LARGE  (~1.2m diam) ──────────────────────────
    {
        auto* s = new Cylinder(0.f, 0.35f, 0.f,
                               0.62f, 0.70f, 12,
                               C_GRANITE_L);
        s->build();
        m_masters[BOULDER_LARGE] = s;
    }

    // ── BOULDER_SANDSTONE  (~1.0m diam, warm tone) ───────────
    {
        auto* s = new Cylinder(0.f, 0.28f, 0.f,
                               0.50f, 0.56f, 10,
                               C_SANDSTONE);
        s->build();
        m_masters[BOULDER_SANDSTONE] = s;
    }

    // ── LIGHT_POLE ───────────────────────────────────────────
    //  Built as a group node, but the registry stores the GROUP
    //  via a DrawableFn that calls two shapes.
    //  Simpler: store as two separate shapes managed via a
    //  sub-node group. We use a Cylinder for the pole and a
    //  small Cylinder for the globe. Since registry only stores
    //  one Shape*, we use the pole as master and add the globe
    //  manually in place(). See place() below.
    {
        auto* s = new Cylinder(0.f, 2.0f, 0.f,   // pole
                               0.05f, 4.0f, 8,
                               C_STEEL);
        s->build();
        m_masters[LIGHT_POLE] = s;
    }

    // ── FENCE_POST ───────────────────────────────────────────
    //  Thin black cylinder; base at y=0 when placed on ground.
    {
        constexpr float postH = 1.0f;
        auto* s = new Cylinder(0.f, postH * 0.5f, 0.f,
                               0.04f, postH, 8,
                               C_FENCE);
        s->build();
        m_masters[FENCE_POST] = s;
    }

    // ── FENCE_RAIL ───────────────────────────────────────────
    //  Horizontal rail: wide, thin, short height.
    //  Scale X to stretch between posts when placing.
    {
        auto* s = new Cuboid(0.f, 0.f, 0.f,
                             0.5f, 0.03f, 0.03f,
                             C_TIMBER);
        s->build();
        m_masters[FENCE_RAIL] = s;
    }

    // ── FENCE_RAIL_BLACK ─────────────────────────────────────
    //  Perimeter rail: 1 m unit length along X, scale to span edge.
    {
        auto* s = new Cuboid(0.f, 0.f, 0.f,
                             0.5f, 0.02f, 0.02f,
                             C_FENCE);
        s->build();
        m_masters[FENCE_RAIL_BLACK] = s;
    }

    // ── FENCE_PICKET ─────────────────────────────────────────
    //  Vertical picket: 0.64 m tall (fits between top/bottom rails).
    {
        constexpr float picketH = 0.64f;
        auto* s = new Cuboid(0.f, picketH * 0.5f, 0.f,
                             0.006f, picketH * 0.5f, 0.006f,
                             C_FENCE);
        s->build();
        m_masters[FENCE_PICKET] = s;
    }

    // ── FLAGPOLE ─────────────────────────────────────────────
    //  Pole at origin — place() adds the banner as a child
    {
        auto* s = new Cylinder(0.f, 0.6f, 0.f,
                               0.025f, 1.2f, 8,
                               C_FLAG_POLE);
        s->build();
        m_masters[FLAGPOLE] = s;
    }

    // ── HOLE_CUP ─────────────────────────────────────────────
    {
        auto* s = new Cylinder(0.f, 0.005f, 0.f,
                               0.09f, 0.04f, 10,
                               C_CUP);
        s->build();
        m_masters[HOLE_CUP] = s;
    }

    // ── SHRUB ────────────────────────────────────────────────
    //  Two cylinders stacked: short wide base + cone top.
    //  We store the base here; cone is added in place().
    {
        auto* s = new Cylinder(0.f, 0.20f, 0.f,
                               0.35f, 0.40f, 10,
                               C_SHRUB);
        s->build();
        m_masters[SHRUB] = s;
    }

    // ── ORNAMENTAL_GRASS ─────────────────────────────────────
    //  Thin tall cylinder — looks like a grass clump from a
    //  distance. Scale Y to vary height when placing.
    {
        auto* s = new Cylinder(0.f, 0.4f, 0.f,
                               0.08f, 0.8f, 6,
                               C_GRASS);
        s->build();
        m_masters[ORNAMENTAL_GRASS] = s;
    }

    // ── NATIVE_TREE ──────────────────────────────────────────
    //  Trunk stored as master; canopy cone added in place().
    {
        auto* s = new Cylinder(0.f, 0.5f, 0.f,
                               0.18f, 1.0f, 8,
                               C_TRUNK);
        s->build();
        m_masters[NATIVE_TREE] = s;
    }

    // ── REED ─────────────────────────────────────────────────
    //  Single aquatic reed: very thin, tall.
    {
        auto* s = new Cylinder(0.f, 0.75f, 0.f,
                               0.02f, 1.5f, 5,
                               C_REED);
        s->build();
        m_masters[REED] = s;
    }

    // ── PATH_EDGE ────────────────────────────────────────────
    //  Low concrete kerb strip: flat wide cuboid.
    //  Scale X when placing to set the length of the kerb.
    {
        auto* s = new Cuboid(0.f, 0.05f, 0.f,
                             0.5f, 0.05f, 0.12f,
                             C_CONCRETE);
        s->build();
        m_masters[PATH_EDGE] = s;
    }
}

// ============================================================
//  clone()  –  returns a new SceneNode sharing the master Shape
// ============================================================
std::shared_ptr<SceneNode> ProtoRegistry::clone(ID id) const
{
    // SceneNode does NOT own/delete its shape pointer, so sharing
    // the same master Shape* across many nodes is safe.
    return std::make_shared<SceneNode>(m_masters[id]);
}

// ============================================================
//  place()  –  clone + transform + optional extra children
// ============================================================
std::shared_ptr<SceneNode> ProtoRegistry::place(
    ID id,
    float tx, float ty, float tz,
    float sx, float sy, float sz,
    float rotY) const
{
    auto node = clone(id);

    // Build transform: T * Ry * S
    Matrix<4,4> T  = makeTranslation3D(tx, ty, tz);
    Matrix<4,4> Ry = makeRotationY(rotY);
    Matrix<4,4> S  = makeScale3D(sx, sy, sz);
    node->localTransform = T * Ry * S;

    // ── Extra children for compound prototypes ───────────────
    //  Some prototypes need a second shape (lamp globe, tree
    //  canopy, shrub cone, flag banner).  We add those here as
    //  children of the placed node.  They are created fresh each
    //  time place() is called — they are NOT prototypes because
    //  they are small and always accompany the parent.

    if(id == LIGHT_POLE)
    {
        // Lamp globe at top of pole (pole is 4.0 tall, cy=2.0)
        auto* globe = new Cylinder(0.f, 4.05f, 0.f,
                                   0.18f, 0.22f, 8,
                                   C_LAMP_HEAD);
        globe->build();
        registerShape(globe);
        node->addChild(std::make_shared<SceneNode>(globe));
    }
    else if(id == FLAGPOLE)
    {
        // Red banner at top (pole cy=0.6, height=1.2 → top at y=1.2)
        auto* banner = new Cuboid(0.10f, 1.12f, 0.f,
                                  0.20f, 0.12f, 0.03f,
                                  C_FLAG_BAN);
        banner->build();
        registerShape(banner);
        node->addChild(std::make_shared<SceneNode>(banner));
    }
    else if(id == SHRUB)
    {
        // Cone top on shrub (base cy=0.2, height=0.4 → top at 0.4)
        auto* top = new Cone(0.f, 0.55f, 0.f,
                             0.32f, 0.45f, 10,
                             C_SHRUB);
        top->build();
        registerShape(top);
        node->addChild(std::make_shared<SceneNode>(top));
    }
    else if(id == NATIVE_TREE)
    {
        // Cone canopy (trunk cy=0.5, height=1.0 → top at 1.0)
        auto* canopy = new Cone(0.f, 1.55f, 0.f,
                                0.55f, 1.1f, 8,
                                C_LEAF);
        canopy->build();
        registerShape(canopy);
        node->addChild(std::make_shared<SceneNode>(canopy));
    }

    return node;
}

// ============================================================
//  cleanup()  –  delete all master shapes
// ============================================================
void ProtoRegistry::cleanup()
{
    for(int i = 0; i < _COUNT; ++i){
        delete m_masters[i];
        m_masters[i] = nullptr;
    }
}