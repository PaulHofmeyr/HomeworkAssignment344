#include "TurfWall.h"
#include <algorithm>
#include <cmath>

TurfWall::TurfWall(float baseY, float height, float r, float g, float b)
    : Shape(r, g, b), m_baseY(baseY), m_height(height)
{}

void TurfWall::newSegment()
{
    m_segments.emplace_back();
}

void TurfWall::addPoint(float x, float z)
{
    if(m_segments.empty())
        newSegment();
    m_segments.back().emplace_back(x, z);
}

void TurfWall::addSegment(const float pts[][2], int count)
{
    if(count <= 0)
        return;
    m_segments.emplace_back();
    m_segments.back().reserve((size_t)count);
    for(int i = 0; i < count; ++i)
        m_segments.back().emplace_back(pts[i][0], pts[i][1]);
}

// Ear-clipping triangulation (same approach as CourseLayout.cpp)
std::vector<float> TurfWall::triangulateFootprint(
    const std::vector<std::pair<float, float>>& poly)
{
    std::vector<float> result;
    const int n = (int)poly.size();
    if(n < 3)
        return result;

    std::vector<float> flat;
    flat.reserve((size_t)n * 2);
    for(const auto& p : poly)
    {
        flat.push_back(p.first);
        flat.push_back(p.second);
    }

    auto getPt = [&](int i) -> std::pair<float, float> {
        return {flat[(size_t)i * 2], flat[(size_t)i * 2 + 1]};
    };

    if(n == 3)
    {
        for(int v = 0; v < 3; ++v)
        {
            auto p = getPt(v);
            result.push_back(p.first);
            result.push_back(p.second);
        }
        return result;
    }

    std::vector<int> idx(n);
    for(int i = 0; i < n; ++i)
        idx[i] = i;

    auto cross2 = [&](int a, int b, int c) -> float {
        auto pa = getPt(a), pb = getPt(b), pc = getPt(c);
        return (pb.first - pa.first) * (pc.second - pb.second)
             - (pb.second - pa.second) * (pc.first - pb.first);
    };
    auto inTri = [&](int p, int a, int b, int c) -> bool {
        float d1 = cross2(a, b, p), d2 = cross2(b, c, p), d3 = cross2(c, a, p);
        return !((d1 < 0 || d2 < 0 || d3 < 0) && (d1 > 0 || d2 > 0 || d3 > 0));
    };

    float area = 0.f;
    for(int i = 0; i < n; ++i)
    {
        int j = (i + 1) % n;
        auto pi = getPt(i), pj = getPt(j);
        area += pi.first * pj.second - pj.first * pi.second;
    }
    if(area < 0.f)
        std::reverse(idx.begin(), idx.end());

    int maxIter = n * n, iter = 0;
    while((int)idx.size() > 3 && iter++ < maxIter)
    {
        bool found = false;
        int sz = (int)idx.size();
        for(int i = 0; i < sz; ++i)
        {
            int prev = idx[(i + sz - 1) % sz];
            int cur  = idx[i];
            int next = idx[(i + 1) % sz];
            if(cross2(prev, cur, next) <= 0.f)
                continue;
            bool ear = true;
            for(int j = 0; j < sz && ear; ++j)
            {
                int v = idx[j];
                if(v == prev || v == cur || v == next)
                    continue;
                if(inTri(v, prev, cur, next))
                    ear = false;
            }
            if(ear)
            {
                for(int v : {prev, cur, next})
                {
                    auto p = getPt(v);
                    result.push_back(p.first);
                    result.push_back(p.second);
                }
                idx.erase(idx.begin() + i);
                found = true;
                break;
            }
        }
        if(!found)
        {
            float cx = 0.f, cz = 0.f;
            for(int v : idx)
            {
                auto p = getPt(v);
                cx += p.first;
                cz += p.second;
            }
            cx /= idx.size();
            cz /= idx.size();
            int sz2 = (int)idx.size();
            for(int i = 0; i < sz2; ++i)
            {
                int j = (i + 1) % sz2;
                auto pi = getPt(idx[i]), pj = getPt(idx[j]);
                result.push_back(cx);
                result.push_back(cz);
                result.push_back(pi.first);
                result.push_back(pi.second);
                result.push_back(pj.first);
                result.push_back(pj.second);
            }
            return result;
        }
    }
    if((int)idx.size() == 3)
    {
        for(int v : idx)
        {
            auto p = getPt(v);
            result.push_back(p.first);
            result.push_back(p.second);
        }
    }
    return result;
}

void TurfWall::extrudePolygon(std::vector<float>& filled,
                              const std::vector<std::pair<float, float>>& poly) const
{
    if(poly.size() < 3)
        return;

    const float y0 = m_baseY;
    const float y1 = m_baseY + m_height;

    std::vector<float> tris = triangulateFootprint(poly);
    const int triCount = (int)tris.size() / 6;

    for(int t = 0; t < triCount; ++t)
        pushTriangle(filled,
                     tris[t * 6 + 0], y1, tris[t * 6 + 1],
                     tris[t * 6 + 2], y1, tris[t * 6 + 3],
                     tris[t * 6 + 4], y1, tris[t * 6 + 5]);

    for(int t = 0; t < triCount; ++t)
        pushTriangle(filled,
                     tris[t * 6 + 0], y0, tris[t * 6 + 1],
                     tris[t * 6 + 4], y0, tris[t * 6 + 5],
                     tris[t * 6 + 2], y0, tris[t * 6 + 3]);

    const int n = (int)poly.size();
    for(int i = 0; i < n; ++i)
    {
        int j = (i + 1) % n;
        float x0 = poly[(size_t)i].first,  z0 = poly[(size_t)i].second;
        float x1 = poly[(size_t)j].first,  z1 = poly[(size_t)j].second;
        pushTriangle(filled, x0, y0, z0, x1, y0, z1, x1, y1, z1);
        pushTriangle(filled, x0, y0, z0, x1, y1, z1, x0, y1, z0);
    }
}

void TurfWall::build()
{
    std::vector<float> filled, wire;

    for(const auto& seg : m_segments)
        extrudePolygon(filled, seg);

    buildBuffers(filled, wire);
}

// BEGIN HOLE07_TURF_WALL_DATA (auto-generated — do not edit by hand)
// Source CSV → world metres (px 1..820, py 33..971 → 40×55 m map)

// 'wall 1' (10 vertices)
static const float hole07_turfWall_seg1[][2] = {
    {   9.597f,   8.385f},
    {   9.548f,   8.854f},
    {  10.330f,   8.971f},
    {  10.965f,   8.913f},
    {  11.404f,   8.854f},
    {  11.746f,   8.678f},
    {  12.088f,   8.443f},
    {  11.893f,   8.092f},
    {  11.306f,   8.326f},
    {  10.427f,   8.385f},
};
static const int hole07_turfWall_seg1Count = 10;

// 'wall 2' (13 vertices)
static const float hole07_turfWall_seg2[][2] = {
    {  11.941f,   6.567f},
    {  11.990f,   5.922f},
    {  11.404f,   5.922f},
    {  10.916f,   5.922f},
    {  10.476f,   5.629f},
    {  10.134f,   5.336f},
    {   9.744f,   4.925f},
    {   9.353f,   4.808f},
    {   9.499f,   5.277f},
    {   9.744f,   5.629f},
    {  10.085f,   5.981f},
    {  10.623f,   6.274f},
    {  11.258f,   6.567f},
};
static const int hole07_turfWall_seg2Count = 13;

// 'wall 3' (11 vertices)
static const float hole07_turfWall_seg3[][2] = {
    {   8.034f,  -0.059f},
    {   8.620f,  -0.117f},
    {   9.206f,  -0.235f},
    {   9.597f,  -0.469f},
    {  10.037f,  -1.055f},
    {  10.232f,  -1.407f},
    {  10.085f,  -1.876f},
    {   9.695f,  -1.173f},
    {   9.304f,  -0.762f},
    {   8.864f,  -0.586f},
    {   8.083f,  -0.528f},
};
static const int hole07_turfWall_seg3Count = 11;

// 'wall 4' (29 vertices)
static const float hole07_turfWall_seg4[][2] = {
    {   7.106f,  -0.235f},
    {   7.155f,  -0.645f},
    {   6.862f,  -0.880f},
    {   6.667f,  -1.173f},
    {   6.374f,  -1.349f},
    {   6.178f,  -1.759f},
    {   6.129f,  -2.170f},
    {   6.129f,  -2.404f},
    {   6.325f,  -2.814f},
    {   6.667f,  -3.166f},
    {   7.057f,  -3.811f},
    {   7.155f,  -4.163f},
    {   7.350f,  -4.574f},
    {   7.692f,  -4.867f},
    {   8.278f,  -4.749f},
    {   8.816f,  -4.339f},
    {   9.255f,  -3.870f},
    {   9.548f,  -3.284f},
    {   9.988f,  -3.401f},
    {   9.841f,  -3.811f},
    {   9.597f,  -4.222f},
    {   8.962f,  -4.749f},
    {   8.230f,  -5.160f},
    {   7.546f,  -5.336f},
    {   6.667f,  -4.691f},
    {   5.934f,  -3.811f},
    {   5.739f,  -2.873f},
    {   5.739f,  -1.876f},
    {   6.129f,  -0.938f},
};
static const int hole07_turfWall_seg4Count = 29;

static void addHole07TurfWallSegments(TurfWall* wall)
{
    wall->addSegment(hole07_turfWall_seg1, hole07_turfWall_seg1Count);
    wall->addSegment(hole07_turfWall_seg2, hole07_turfWall_seg2Count);
    wall->addSegment(hole07_turfWall_seg3, hole07_turfWall_seg3Count);
    wall->addSegment(hole07_turfWall_seg4, hole07_turfWall_seg4Count);
}
// END HOLE07_TURF_WALL_DATA

TurfWall* TurfWall::createHole07()
{
    constexpr float Y_BASE = 0.007f;
    constexpr float HEIGHT = 0.32f;
    constexpr float R = 0.12f, G = 0.50f, B = 0.14f;

    auto* wall = new TurfWall(Y_BASE, HEIGHT, R, G, B);
    addHole07TurfWallSegments(wall);
    wall->build();
    return wall;
}
