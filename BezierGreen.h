#ifndef BEZIER_GREEN_H
#define BEZIER_GREEN_H

#include "Shape.h"
#include <vector>

// ============================================================
//  BezierGreen
//
//  A flat horizontal polygon whose outline is defined by a
//  sequence of cubic Bézier curve segments.  The result is
//  triangulated using a fan from the centroid and rendered
//  as a flat slab at a given Y height.
//
//  Usage:
//    BezierGreen *g = new BezierGreen(y, r, g, b);
//    g->addCubic(p0x,p0z, c1x,c1z, c2x,c2z, p1x,p1z);
//    ... (add more segments, last point of one = first of next)
//    g->build();
// ============================================================

struct BezPt { float x, z; };

class BezierGreen : public Shape
{
public:
    BezierGreen(float y, float r, float g, float b);

    // Add one cubic Bézier segment.
    // p0 = start, c1/c2 = control points, p1 = end.
    // The outline is built by connecting all segments in order.
    void addCubic(float p0x, float p0z,
                  float c1x, float c1z,
                  float c2x, float c2z,
                  float p1x, float p1z,
                  int   steps = 24);

    void build() override;

private:
    float m_y;
    std::vector<BezPt> m_outline;  // sampled boundary points

    // Ear-clip triangulation (works for simple polygons)
    void triangulate(std::vector<float> &filled,
                     std::vector<float> &wire);
};

#endif // BEZIER_GREEN_H