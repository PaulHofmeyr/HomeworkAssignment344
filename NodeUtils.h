#ifndef NODEUTILS_H
#define NODEUTILS_H

// ============================================================
//  NodeUtils.h
//
//  Tiny helpers shared by all node-builder files so nobody
//  duplicates the same boilerplate.
// ============================================================

#include <memory>
#include <vector>
#include "SceneNode.h"
#include "DrawableFn.h"
#include "Cylinder.h"
#include "Cuboid.h"
#include "Cone.h"
#include "TriangularPrism.h"
#include "Transformations.h"

// ── Shape registry ──────────────────────────────────────────
//  Every node file calls registerShape() so the master cleanup
//  can delete everything in one place (see SceneRoot.cpp).
extern std::vector<Shape*> g_allShapes;

inline void registerShape(Shape* s) { g_allShapes.push_back(s); }

// ── Helper: wrap a zero-arg draw function as a SceneNode leaf ─
inline std::shared_ptr<SceneNode> fnNode(std::function<void()> fn)
{
    auto* s = new DrawableFn(fn);
    registerShape(s);
    return std::make_shared<SceneNode>(s);
}

// ── Helper: wrap a heap-allocated Shape in a SceneNode leaf ──
//  Registers the shape for later deletion.
inline std::shared_ptr<SceneNode> shapeNode(Shape* s)
{
    registerShape(s);
    return std::make_shared<SceneNode>(s);
}

// ── Helper: build a Shape on the heap and register it ────────
template<typename T, typename... Args>
T* makeShape(Args&&... args)
{
    T* s = new T(std::forward<Args>(args)...);
    registerShape(s);
    return s;
}

#endif // NODEUTILS_H
