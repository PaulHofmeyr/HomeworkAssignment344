#include "BezierGreen.h"
#include <cmath>
#include <vector>
#include <algorithm>

// ── Constructor ──────────────────────────────────────────────
BezierGreen::BezierGreen(float y, float r, float g, float b)
    : Shape(r, g, b), m_y(y)
{}

// ── addCubic ─────────────────────────────────────────────────
// Sample a cubic Bézier curve and append points to m_outline.
// B(t) = (1-t)³P0 + 3(1-t)²tC1 + 3(1-t)t²C2 + t³P1
void BezierGreen::addCubic(float p0x, float p0z,
                            float c1x, float c1z,
                            float c2x, float c2z,
                            float p1x, float p1z,
                            int   steps)
{
    for (int i = 0; i <= steps; i++)
    {
        // Skip the first point on all but the very first segment
        // to avoid duplicating junction points.
        if (i == 0 && !m_outline.empty()) continue;

        float t  = (float)i / (float)steps;
        float mt = 1.0f - t;

        float x = mt*mt*mt*p0x
                + 3.0f*mt*mt*t*c1x
                + 3.0f*mt*t*t*c2x
                + t*t*t*p1x;

        float z = mt*mt*mt*p0z
                + 3.0f*mt*mt*t*c1z
                + 3.0f*mt*t*t*c2z
                + t*t*t*p1z;

        m_outline.push_back({x, z});
    }
}

// ── build ─────────────────────────────────────────────────────
void BezierGreen::build()
{
    std::vector<float> filled, wire;
    triangulate(filled, wire);
    buildBuffers(filled, wire);
}

// ── triangulate ───────────────────────────────────────────────
// Simple centroid fan triangulation.
// Works perfectly for convex or mildly concave outlines
// (which all our greens are).
void BezierGreen::triangulate(std::vector<float> &filled,
                               std::vector<float> &wire)
{
    if (m_outline.size() < 3) return;

    // Compute centroid
    float cx = 0, cz = 0;
    for (auto &p : m_outline) { cx += p.x; cz += p.z; }
    cx /= (float)m_outline.size();
    cz /= (float)m_outline.size();

    const float ny = 1.0f; // flat upward normal
    const float y  = m_y;

    size_t n = m_outline.size();

    for (size_t i = 0; i < n; i++)
    {
        size_t j = (i + 1) % n;

        float ax = m_outline[i].x, az = m_outline[i].z;
        float bx = m_outline[j].x, bz = m_outline[j].z;

        // Fan triangle: centroid → a → b
        pushTriangle(filled,
                     cx, y, cz,
                     ax, y, az,
                     bx, y, bz);

        // Wireframe edge: a → b
        pushLine(wire, ax, y, az, bx, y, bz);
    }
}