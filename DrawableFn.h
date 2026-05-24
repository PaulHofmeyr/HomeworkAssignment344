#ifndef DRAWABLEFN_H
#define DRAWABLEFN_H

// ============================================================
//  DrawableFn.h
//
//  A thin Shape subclass that wraps any std::function<void()>
//  as a drawable leaf.  This lets FlatPoly / Disc objects that
//  live inside CourseLayout be plugged into a SceneNode without
//  changing CourseLayout at all.
//
//  Usage:
//      auto* s = new DrawableFn([](){ myFlatPoly.draw(); });
//      auto node = std::make_shared<SceneNode>(s);
// ============================================================

#include "Shape.h"
#include <functional>

class DrawableFn : public Shape
{
    std::function<void()> m_fn;
public:
    explicit DrawableFn(std::function<void()> fn)
        : Shape(1,1,1), m_fn(fn) {}

    void build()         override {}          // geometry lives in CourseLayout
    void drawFilled()    const override { m_fn(); }
    void drawWireframe() const override { m_fn(); }
};

#endif // DRAWABLEFN_H
