#ifndef TURFWALL_H
#define TURFWALL_H

#include "Shape.h"
#include <utility>
#include <vector>

// ============================================================
//  TurfWall
//
//  Raised turf berm built from closed polygon outlines.
//  Each addSegment() takes ordered vertices (your map clicks)
//  tracing one wall piece; the footprint is triangulated and
//  extruded upward so curved outlines stay curved, not strips.
// ============================================================

class TurfWall : public Shape
{
public:
    TurfWall(float baseY, float height, float r, float g, float b);

    void newSegment();
    void addPoint(float x, float z);
    void addSegment(const float pts[][2], int count);

    void build() override;

    static TurfWall* createHole07();

private:
    float m_baseY;
    float m_height;
    std::vector<std::vector<std::pair<float, float>>> m_segments;

    static std::vector<float> triangulateFootprint(
        const std::vector<std::pair<float, float>>& poly);

    void extrudePolygon(std::vector<float>& filled,
                        const std::vector<std::pair<float, float>>& poly) const;
};

#endif // TURFWALL_H
