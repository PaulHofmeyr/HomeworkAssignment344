#ifndef PROTOTYPE_H
#define PROTOTYPE_H

// ============================================================
//  Prototype.h  –  Prototype Pattern for reusable scene objects
//
//  THE IDEA
//  ────────
//  Building geometry (uploading to GPU) is expensive.
//  Instead of building 50 boulders, we build ONE master Shape
//  ("prototype"), then every clone() just shares the same
//  Shape* pointer and gets its OWN SceneNode with its own
//  localTransform.  Zero extra GPU work per clone.
//
//  HOW IT WORKS
//  ────────────
//  • Each prototype is a real Shape (Cylinder/Cuboid/Cone/etc)
//    built once and stored in ProtoRegistry.
//  • clone(id) creates a new SceneNode pointing at that Shape.
//  • place(id, x,y,z, sx,sy,sz, rotY) clones + sets transform.
//  • Multiple SceneNodes can safely share one Shape* because
//    SceneNode never deletes its shape pointer — ProtoRegistry
//    owns and deletes all master shapes at cleanup().
//
//  USAGE EXAMPLE
//  ─────────────
//  // In any node .cpp file:
//  auto& R = ProtoRegistry::get();
//
//  // Place a large boulder at world position (5, 0, -3)
//  holeNode->addChild(
//      R.place(ProtoRegistry::BOULDER_LARGE, 5.f, 0.f, -3.f));
//
//  // Place a lamp post, rotated 45 degrees, scaled taller
//  holeNode->addChild(
//      R.place(ProtoRegistry::LIGHT_POLE,
//              10.f, 0.f, 2.f,       // position
//              1.f, 1.5f, 1.f,       // scale (taller)
//              0.785f));             // rotY = 45 deg
//
//  PROTOTYPE IDs
//  ─────────────
//  BOULDER_SMALL       ~0.3m grey granite disc
//  BOULDER_MED         ~0.7m grey granite disc
//  BOULDER_LARGE       ~1.2m grey granite disc
//  BOULDER_SANDSTONE   ~1.0m warm sandstone disc
//  LIGHT_POLE          lamp-post pole + globe head
//  FENCE_POST          thin black cylinder (perimeter fence)
//  FENCE_RAIL          horizontal wooden rail (hole sections)
//  FENCE_RAIL_BLACK    thin black horizontal rail (perimeter)
//  FENCE_PICKET        very thin black vertical picket (perimeter)
//  FLAGPOLE            metal pole + red banner
//  HOLE_CUP            dark flat cylinder (cup)
//  SHRUB               green hemisphere shrub
//  ORNAMENTAL_GRASS    tall thin grass clump
//  NATIVE_TREE         trunk cylinder + cone canopy
//  REED                thin aquatic reed
//  PATH_EDGE           low concrete kerb strip
// ============================================================

#include <memory>
#include <vector>
#include <GL/glew.h>
#include "SceneNode.h"
#include "Transformations.h"
#include "Shape.h"

class ProtoRegistry
{
public:
    // ── Prototype IDs ────────────────────────────────────────
    enum ID {
        BOULDER_SMALL = 0,
        BOULDER_MED,
        BOULDER_LARGE,
        BOULDER_SANDSTONE,
        LIGHT_POLE,
        FENCE_POST,
        FENCE_RAIL,
        FENCE_RAIL_BLACK,
        FENCE_PICKET,
        FLAGPOLE,
        HOLE_CUP,
        SHRUB,
        ORNAMENTAL_GRASS,
        NATIVE_TREE,
        REED,
        PATH_EDGE,
        _COUNT
    };

    // Singleton access
    static ProtoRegistry& get() {
        static ProtoRegistry inst;
        return inst;
    }

    // Build all prototype master shapes — call once after GL init
    void build();

    // ── place() ──────────────────────────────────────────────
    //  Clone a prototype and apply position + scale + Y-rotation.
    //  This is the main function you call from node files.
    //
    //  tx,ty,tz  = world position
    //  sx,sy,sz  = scale (default 1,1,1 = no scale)
    //  rotY      = Y-axis rotation in radians (default 0)
    std::shared_ptr<SceneNode> place(
        ID    id,
        float tx, float ty, float tz,
        float sx = 1.f, float sy = 1.f, float sz = 1.f,
        float rotY = 0.f
    ) const;

    // ── clone() ──────────────────────────────────────────────
    //  Clone with identity transform — set node->localTransform
    //  yourself afterwards.
    std::shared_ptr<SceneNode> clone(ID id) const;

    // Free all master shapes (call at program exit)
    void cleanup();

private:
    ProtoRegistry() = default;
    Shape* m_masters[_COUNT] = {};
};

#endif // PROTOTYPE_H
