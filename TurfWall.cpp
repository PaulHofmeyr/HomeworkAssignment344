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

// BEGIN HOLE14_TURF_WALL_DATA (auto-generated — do not edit by hand)
// Source: hole 14.csv

// 'hole 14 turf wall' (16 vertices)
static const float hole14_turfWall_seg1[][2] = {
    {  -7.888f,  22.047f},
    {  -7.546f,  21.636f},
    {  -7.741f,  21.109f},
    {  -7.985f,  20.640f},
    {  -7.985f,  20.112f},
    {  -7.643f,  19.467f},
    {  -7.302f,  18.822f},
    {  -6.813f,  18.353f},
    {  -6.862f,  17.884f},
    {  -7.253f,  17.942f},
    {  -7.741f,  18.294f},
    {  -8.034f,  18.822f},
    {  -8.230f,  19.467f},
    {  -8.474f,  20.346f},
    {  -8.376f,  21.050f},
    {  -8.034f,  21.754f},
};
static const int hole14_turfWall_seg1Count = 16;

// 'wall 2' (27 vertices)
static const float hole14_turfWall_seg2[][2] = {
    {  -5.885f,  22.985f},
    {  -6.178f,  22.985f},
    {  -6.178f,  23.337f},
    {  -5.495f,  23.982f},
    {  -4.713f,  24.510f},
    {  -3.834f,  25.037f},
    {  -2.906f,  25.272f},
    {  -2.027f,  25.506f},
    {  -1.148f,  25.506f},
    {  -0.220f,  25.624f},
    {   0.513f,  25.624f},
    {   1.050f,  25.624f},
    {   1.538f,  25.506f},
    {   1.880f,  25.506f},
    {   1.880f,  25.037f},
    {   1.343f,  25.037f},
    {   0.806f,  25.037f},
    {   0.317f,  25.096f},
    {  -0.220f,  25.096f},
    {  -0.806f,  25.096f},
    {  -1.392f,  25.096f},
    {  -1.929f,  24.744f},
    {  -2.613f,  24.510f},
    {  -3.346f,  24.275f},
    {  -4.078f,  23.865f},
    {  -4.664f,  23.513f},
    {  -5.299f,  23.396f},
};
static const int hole14_turfWall_seg2Count = 27;

// 'wall 3' (10 vertices)
static const float hole14_turfWall_seg3[][2] = {
    {   2.418f,  22.751f},
    {   2.906f,  22.633f},
    {   3.443f,  22.751f},
    {   3.736f,  23.102f},
    {   4.078f,  23.396f},
    {   4.078f,  23.865f},
    {   3.883f,  24.216f},
    {   3.492f,  23.923f},
    {   3.199f,  23.571f},
    {   2.808f,  23.220f},
};
static const int hole14_turfWall_seg3Count = 10;

static void addHole14TurfWallSegments(TurfWall* wall)
{
    wall->addSegment(hole14_turfWall_seg1, hole14_turfWall_seg1Count);
    wall->addSegment(hole14_turfWall_seg2, hole14_turfWall_seg2Count);
    wall->addSegment(hole14_turfWall_seg3, hole14_turfWall_seg3Count);
}
// END HOLE14_TURF_WALL_DATA

// BEGIN HOLE16_TURF_WALL_DATA (auto-generated — do not edit by hand)
// Source: hole-16-turf-wall.csv

// 'hole 14 turf wall' (66 vertices)
static const float hole16_turfWall_seg1[][2] = {
    { -14.042f,  17.063f},
    { -13.651f,  17.532f},
    { -13.016f,  17.122f},
    { -12.332f,  16.652f},
    { -11.746f,  16.242f},
    { -11.258f,  15.890f},
    { -10.867f,  15.421f},
    { -10.134f,  14.190f},
    {  -9.695f,  13.193f},
    {  -9.402f,  12.548f},
    {  -9.402f,  11.786f},
    {  -9.646f,  10.848f},
    {  -9.890f,   9.675f},
    { -10.085f,   8.971f},
    { -10.476f,   8.502f},
    { -10.720f,   7.974f},
    { -11.306f,   7.271f},
    { -11.697f,   6.919f},
    { -12.039f,   6.509f},
    { -12.430f,   6.333f},
    { -12.772f,   5.981f},
    { -13.309f,   5.746f},
    { -13.602f,   5.453f},
    { -14.139f,   5.160f},
    { -14.579f,   5.101f},
    { -14.969f,   5.629f},
    { -15.263f,   6.215f},
    { -15.604f,   7.036f},
    { -15.849f,   7.974f},
    { -16.093f,   8.678f},
    { -16.239f,   9.440f},
    { -16.142f,  10.261f},
    { -15.849f,  10.965f},
    { -15.751f,  11.434f},
    { -15.653f,  12.196f},
    { -15.214f,  12.196f},
    { -15.214f,  11.668f},
    { -15.116f,  11.082f},
    { -15.458f,  10.613f},
    { -15.604f,   9.968f},
    { -15.604f,   9.147f},
    { -15.458f,   8.385f},
    { -15.360f,   7.799f},
    { -15.165f,   7.388f},
    { -15.018f,   6.802f},
    { -14.676f,   6.274f},
    { -14.286f,   5.981f},
    { -13.797f,   5.981f},
    { -13.162f,   6.274f},
    { -12.576f,   6.743f},
    { -11.990f,   7.212f},
    { -11.453f,   7.799f},
    { -10.916f,   8.561f},
    { -10.574f,   9.147f},
    { -10.232f,   9.675f},
    {  -9.988f,  10.496f},
    {  -9.988f,  11.317f},
    {  -9.988f,  12.138f},
    { -10.134f,  12.783f},
    { -10.379f,  13.486f},
    { -10.818f,  14.190f},
    { -11.209f,  14.776f},
    { -11.551f,  15.069f},
    { -12.039f,  15.773f},
    { -12.674f,  16.183f},
    { -13.358f,  16.652f},
};
static const int hole16_turfWall_seg1Count = 66;

// 'wall 2' (15 vertices)
static const float hole16_turfWall_seg2[][2] = {
    { -15.458f,  15.245f},
    { -15.458f,  13.779f},
    { -15.214f,  13.486f},
    { -14.823f,  13.603f},
    { -14.481f,  13.662f},
    { -14.188f,  13.838f},
    { -13.944f,  13.838f},
    { -13.602f,  13.779f},
    { -12.869f,  13.486f},
    { -12.283f,  13.428f},
    { -12.283f,  14.014f},
    { -13.114f,  14.542f},
    { -13.602f,  14.952f},
    { -14.237f,  15.011f},
    { -14.725f,  15.128f},
};
static const int hole16_turfWall_seg2Count = 15;

// 'wall 3' (12 vertices)
static const float hole16_turfWall_seg3[][2] = {
    { -14.042f,  10.496f},
    { -14.139f,   9.909f},
    { -13.993f,   9.264f},
    { -13.602f,   8.737f},
    { -12.918f,   8.737f},
    { -12.381f,   8.737f},
    { -11.697f,   9.264f},
    { -11.844f,   9.499f},
    { -12.332f,   9.499f},
    { -12.674f,   9.499f},
    { -13.016f,   9.675f},
    { -13.504f,  10.144f},
};
static const int hole16_turfWall_seg3Count = 12;

static void addHole16TurfWallSegments(TurfWall* wall)
{
    wall->addSegment(hole16_turfWall_seg1, hole16_turfWall_seg1Count);
    wall->addSegment(hole16_turfWall_seg2, hole16_turfWall_seg2Count);
    wall->addSegment(hole16_turfWall_seg3, hole16_turfWall_seg3Count);
}
// END HOLE16_TURF_WALL_DATA

// BEGIN HOLE15_TURF_WALL_DATA (auto-generated — do not edit by hand)
// Source: hole-15-turf-wall.csv

// 'hole 15 turf wall' — lower inner berm (31 vertices)
static const float hole15_turfWall_seg1[][2] = {
    { -16.288f,  17.063f},
    { -16.581f,  16.828f},
    { -16.581f,  17.180f},
    { -16.630f,  17.884f},
    { -16.728f,  18.470f},
    { -16.532f,  18.998f},
    { -16.142f,  18.998f},
    { -15.653f,  19.057f},
    { -15.116f,  18.998f},
    { -14.579f,  18.998f},
    { -14.188f,  18.998f},
    { -13.504f,  19.291f},
    { -12.967f,  19.584f},
    { -12.137f,  19.936f},
    { -11.746f,  20.288f},
    { -11.404f,  20.874f},
    { -10.965f,  21.167f},
    { -10.623f,  21.285f},
    { -10.379f,  21.461f},
    {  -9.597f,  21.871f},
    {  -9.255f,  21.578f},
    { -10.183f,  20.640f},
    { -10.916f,  19.936f},
    { -11.551f,  19.291f},
    { -12.137f,  18.998f},
    { -12.918f,  18.587f},
    { -13.797f,  18.529f},
    { -14.676f,  18.470f},
    { -15.214f,  18.470f},
    { -15.653f,  18.177f},
    { -15.995f,  17.532f},
};
static const int hole15_turfWall_seg1Count = 31;

// 'wall 2' — upper outer berm (27 vertices)
static const float hole15_turfWall_seg2[][2] = {
    { -20.049f,  17.884f},
    { -19.707f,  17.884f},
    { -19.658f,  17.356f},
    { -19.658f,  16.946f},
    { -19.658f,  16.535f},
    { -19.658f,  15.773f},
    { -19.316f,  14.776f},
    { -18.730f,  14.659f},
    { -18.242f,  14.659f},
    { -17.900f,  14.600f},
    { -17.509f,  14.659f},
    { -17.265f,  14.893f},
    { -17.021f,  15.245f},
    { -16.679f,  15.245f},
    { -16.630f,  15.011f},
    { -16.972f,  14.600f},
    { -17.314f,  14.131f},
    { -18.046f,  14.072f},
    { -18.584f,  14.014f},
    { -19.219f,  14.131f},
    { -19.560f,  14.366f},
    { -19.853f,  14.893f},
    { -20.000f,  15.421f},
    { -20.000f,  16.007f},
    { -20.000f,  16.535f},
    { -20.049f,  17.004f},
    { -20.049f,  17.473f},
};
static const int hole15_turfWall_seg2Count = 27;

// 'wall 3' — left end berm (33 vertices)
static const float hole15_turfWall_seg3[][2] = {
    { -15.507f,  22.047f},
    { -15.165f,  21.636f},
    { -14.432f,  21.343f},
    { -13.944f,  21.285f},
    { -13.309f,  21.343f},
    { -12.821f,  21.519f},
    { -12.430f,  21.695f},
    { -11.893f,  21.930f},
    { -11.551f,  22.457f},
    { -11.258f,  23.161f},
    { -10.916f,  23.454f},
    { -10.476f,  23.865f},
    { -10.037f,  24.099f},
    {  -9.597f,  24.099f},
    {  -9.060f,  23.806f},
    {  -8.864f,  23.571f},
    {  -8.571f,  22.926f},
    {  -8.034f,  22.926f},
    {  -8.083f,  23.278f},
    {  -8.718f,  24.041f},
    {  -9.353f,  24.510f},
    {  -9.890f,  24.627f},
    { -10.379f,  24.510f},
    { -10.916f,  24.275f},
    { -11.258f,  23.982f},
    { -11.648f,  23.396f},
    { -11.941f,  22.751f},
    { -12.332f,  22.340f},
    { -12.723f,  22.223f},
    { -13.358f,  22.223f},
    { -14.090f,  22.164f},
    { -14.725f,  22.164f},
    { -15.165f,  22.223f},
};
static const int hole15_turfWall_seg3Count = 33;

// 'wall 4' — island obstacle (12 vertices)
static const float hole15_turfWall_seg4[][2] = {
    { -18.486f,  20.522f},
    { -18.046f,  20.816f},
    { -17.607f,  20.816f},
    { -17.265f,  20.405f},
    { -16.972f,  19.995f},
    { -16.923f,  19.350f},
    { -17.118f,  18.763f},
    { -17.460f,  18.822f},
    { -17.802f,  19.057f},
    { -18.193f,  19.291f},
    { -18.437f,  19.643f},
    { -18.584f,  20.053f},
};
static const int hole15_turfWall_seg4Count = 12;

static void addHole15TurfWallSegments(TurfWall* wall)
{
    wall->addSegment(hole15_turfWall_seg1, hole15_turfWall_seg1Count);
    wall->addSegment(hole15_turfWall_seg2, hole15_turfWall_seg2Count);
    wall->addSegment(hole15_turfWall_seg3, hole15_turfWall_seg3Count);
    wall->addSegment(hole15_turfWall_seg4, hole15_turfWall_seg4Count);
}
// END HOLE15_TURF_WALL_DATA

// BEGIN HOLE13_TURF_WALL_DATA (auto-generated — do not edit by hand)
// Source: hole-13-turf-wall.csv

// 'hole 13 turf wall' (72 vertices)
static const float hole13_turfWall_seg1[][2] = {
    {   1.392f,  11.962f},
    {   1.001f,  12.138f},
    {   0.904f,  11.551f},
    {   0.952f,  10.965f},
    {   1.099f,  10.554f},
    {   1.538f,  10.027f},
    {   1.929f,   9.851f},
    {   2.613f,   9.851f},
    {   3.150f,   9.968f},
    {   3.492f,  10.027f},
    {   3.785f,  10.085f},
    {   4.371f,  10.261f},
    {   4.664f,  10.437f},
    {   4.957f,  10.730f},
    {   5.348f,  10.906f},
    {   5.641f,  11.141f},
    {   5.983f,  11.375f},
    {   7.057f,  12.196f},
    {   7.546f,  12.724f},
    {   8.083f,  13.486f},
    {   8.474f,  14.248f},
    {   8.913f,  15.011f},
    {   9.206f,  16.359f},
    {   9.451f,  17.649f},
    {   9.646f,  18.998f},
    {   9.744f,  20.053f},
    {   9.744f,  21.402f},
    {   9.206f,  22.223f},
    {   8.474f,  22.809f},
    {   7.839f,  23.044f},
    {   7.204f,  22.985f},
    {   6.618f,  22.868f},
    {   6.032f,  22.575f},
    {   5.788f,  22.399f},
    {   5.250f,  21.812f},
    {   5.104f,  21.050f},
    {   5.104f,  20.405f},
    {   5.397f,  20.346f},
    {   5.446f,  21.109f},
    {   5.836f,  21.754f},
    {   6.178f,  22.106f},
    {   6.667f,  22.340f},
    {   7.057f,  22.457f},
    {   7.399f,  22.457f},
    {   7.790f,  22.399f},
    {   8.181f,  22.399f},
    {   8.669f,  22.106f},
    {   9.158f,  21.519f},
    {   9.304f,  21.109f},
    {   9.353f,  20.405f},
    {   9.353f,  19.643f},
    {   9.304f,  18.939f},
    {   9.109f,  18.236f},
    {   8.962f,  17.239f},
    {   8.864f,  16.535f},
    {   8.571f,  15.714f},
    {   8.181f,  14.542f},
    {   7.741f,  13.897f},
    {   7.399f,  13.428f},
    {   6.911f,  12.783f},
    {   6.374f,  12.372f},
    {   5.739f,  11.786f},
    {   5.055f,  11.610f},
    {   4.322f,  11.493f},
    {   3.590f,  11.141f},
    {   3.101f,  10.789f},
    {   2.857f,  10.613f},
    {   2.515f,  10.554f},
    {   2.125f,  10.496f},
    {   1.783f,  10.496f},
    {   1.490f,  10.965f},
    {   1.392f,  11.434f},
};
static const int hole13_turfWall_seg1Count = 72;

// 'wall 2' (13 vertices)
static const float hole13_turfWall_seg2[][2] = {
    {   2.662f,  13.252f},
    {   2.759f,  12.841f},
    {   3.150f,  13.193f},
    {   3.394f,  13.486f},
    {   3.883f,  13.955f},
    {   3.980f,  14.483f},
    {   4.029f,  14.835f},
    {   4.225f,  15.128f},
    {   4.127f,  15.597f},
    {   3.736f,  15.187f},
    {   3.639f,  14.893f},
    {   3.346f,  14.307f},
    {   3.004f,  13.897f},
};
static const int hole13_turfWall_seg2Count = 13;

// 'wall' (38 vertices)
static const float hole13_turfWall_seg3[][2] = {
    {   5.885f,  13.838f},
    {   6.081f,  14.307f},
    {   6.276f,  14.776f},
    {   6.471f,  15.128f},
    {   6.764f,  15.480f},
    {   6.667f,  15.832f},
    {   6.178f,  16.007f},
    {   5.592f,  16.301f},
    {   4.957f,  16.594f},
    {   4.322f,  16.770f},
    {   3.883f,  16.594f},
    {   3.541f,  16.535f},
    {   3.004f,  16.125f},
    {   2.271f,  15.538f},
    {   1.685f,  15.421f},
    {   1.685f,  15.832f},
    {   2.125f,  16.125f},
    {   2.662f,  16.477f},
    {   3.150f,  16.946f},
    {   3.541f,  17.239f},
    {   3.883f,  17.532f},
    {   4.274f,  18.060f},
    {   4.518f,  18.587f},
    {   4.860f,  18.646f},
    {   4.860f,  18.294f},
    {   4.762f,  17.825f},
    {   4.811f,  17.356f},
    {   5.055f,  17.122f},
    {   5.495f,  16.946f},
    {   5.739f,  16.770f},
    {   6.178f,  16.652f},
    {   6.667f,  16.359f},
    {   7.155f,  16.066f},
    {   7.253f,  15.362f},
    {   7.155f,  14.835f},
    {   6.960f,  14.248f},
    {   6.618f,  13.897f},
    {   6.227f,  13.603f},
};
static const int hole13_turfWall_seg3Count = 38;

static void addHole13TurfWallSegments(TurfWall* wall)
{
    wall->addSegment(hole13_turfWall_seg1, hole13_turfWall_seg1Count);
    wall->addSegment(hole13_turfWall_seg2, hole13_turfWall_seg2Count);
    wall->addSegment(hole13_turfWall_seg3, hole13_turfWall_seg3Count);
}
// END HOLE13_TURF_WALL_DATA

// BEGIN HOLE12_TURF_WALL_DATA (auto-generated — do not edit by hand)
// Source: hole-12-turf-wall.csv
// px 1..820, py 33..971 → 40×55 m map

// 'holee 12 turf wall' (72 vertices)
static const float hole12_turfWall_seg1[][2] = {
    {  -5.299f,  12.313f},
    {  -5.495f,  11.903f},
    {  -6.032f,  11.903f},
    {  -6.667f,  12.665f},
    {  -7.204f,  13.369f},
    {  -7.350f,  14.131f},
    {  -7.350f,  14.893f},
    {  -7.302f,  15.773f},
    {  -7.057f,  16.242f},
    {  -6.667f,  17.063f},
    {  -6.276f,  17.532f},
    {  -6.032f,  17.591f},
    {  -5.397f,  18.353f},
    {  -4.860f,  18.822f},
    {  -4.420f,  19.291f},
    {  -3.883f,  19.526f},
    {  -3.297f,  19.995f},
    {  -2.857f,  20.464f},
    {  -2.027f,  20.581f},
    {  -1.587f,  20.581f},
    {  -1.197f,  20.933f},
    {  -0.464f,  21.461f},
    {   0.269f,  21.871f},
    {   1.050f,  22.106f},
    {   1.734f,  22.281f},
    {   2.173f,  22.106f},
    {   2.564f,  21.754f},
    {   3.053f,  21.461f},
    {   3.297f,  21.167f},
    {   3.590f,  20.757f},
    {   3.785f,  20.112f},
    {   3.785f,  19.584f},
    {   3.736f,  19.174f},
    {   3.590f,  18.705f},
    {   3.492f,  18.470f},
    {   3.101f,  18.412f},
    {   3.101f,  18.763f},
    {   3.297f,  19.232f},
    {   3.346f,  20.053f},
    {   3.150f,  20.757f},
    {   2.759f,  21.167f},
    {   2.271f,  21.578f},
    {   1.783f,  21.754f},
    {   1.099f,  21.754f},
    {   0.562f,  21.578f},
    {   0.073f,  21.285f},
    {  -0.269f,  21.109f},
    {  -0.611f,  20.874f},
    {  -1.001f,  20.522f},
    {  -1.197f,  20.288f},
    {  -1.734f,  20.112f},
    {  -3.150f,  19.643f},
    {  -3.639f,  19.467f},
    {  -3.932f,  19.232f},
    {  -4.371f,  18.822f},
    {  -4.713f,  18.529f},
    {  -4.957f,  18.236f},
    {  -5.299f,  18.001f},
    {  -5.592f,  17.591f},
    {  -6.081f,  17.239f},
    {  -6.325f,  16.887f},
    {  -6.569f,  16.594f},
    {  -6.667f,  16.301f},
    {  -6.862f,  15.890f},
    {  -6.862f,  15.421f},
    {  -6.862f,  14.952f},
    {  -6.862f,  14.307f},
    {  -6.813f,  13.897f},
    {  -6.618f,  13.428f},
    {  -6.276f,  12.958f},
    {  -6.032f,  12.841f},
    {  -5.446f,  12.783f},
};
static const int hole12_turfWall_seg1Count = 72;

// 'wall 2' (37 vertices)
static const float hole12_turfWall_seg2[][2] = {
    {  -4.127f,  14.307f},
    {  -4.029f,  13.955f},
    {  -3.834f,  13.955f},
    {  -3.590f,  14.248f},
    {  -3.346f,  14.542f},
    {  -3.053f,  15.245f},
    {  -2.711f,  16.007f},
    {  -2.173f,  16.594f},
    {  -1.783f,  17.239f},
    {  -1.294f,  17.767f},
    {  -0.855f,  18.353f},
    {  -0.366f,  18.412f},
    {   0.024f,  18.236f},
    {   0.464f,  17.942f},
    {   0.904f,  17.591f},
    {   1.294f,  17.473f},
    {   1.587f,  17.356f},
    {   2.125f,  17.356f},
    {   2.076f,  17.884f},
    {   1.783f,  17.884f},
    {   1.294f,  18.060f},
    {   0.952f,  18.412f},
    {   0.806f,  18.939f},
    {   0.317f,  19.232f},
    {  -0.220f,  19.115f},
    {  -0.708f,  19.115f},
    {  -1.245f,  19.174f},
    {  -1.636f,  19.174f},
    {  -2.271f,  19.057f},
    {  -2.613f,  18.470f},
    {  -2.955f,  18.177f},
    {  -3.150f,  17.649f},
    {  -3.248f,  16.946f},
    {  -3.346f,  16.535f},
    {  -3.346f,  16.007f},
    {  -3.492f,  15.421f},
    {  -3.785f,  14.835f},
};
static const int hole12_turfWall_seg2Count = 37;

static void addHole12TurfWallSegments(TurfWall* wall)
{
    wall->addSegment(hole12_turfWall_seg1, hole12_turfWall_seg1Count);
    wall->addSegment(hole12_turfWall_seg2, hole12_turfWall_seg2Count);
}
// END HOLE12_TURF_WALL_DATA

// BEGIN HOLE11_TURF_WALL_DATA (auto-generated — do not edit by hand)
// Source: hole-11-turf-wall.csv
// px 1..820, py 33..971 → 40×55 m map

// 'hole 11 turf wall' (88 vertices)
static const float hole11_turfWall_seg1[][2] = {
    {  -9.402f,   4.339f},
    {  -9.060f,   4.691f},
    {  -8.816f,   4.339f},
    {  -8.376f,   3.987f},
    {  -8.034f,   3.811f},
    {  -7.497f,   3.811f},
    {  -7.057f,   4.046f},
    {  -6.716f,   4.398f},
    {  -6.422f,   4.515f},
    {  -6.081f,   4.867f},
    {  -5.641f,   5.043f},
    {  -5.201f,   5.160f},
    {  -4.811f,   5.219f},
    {  -4.420f,   5.336f},
    {  -3.932f,   5.570f},
    {  -3.443f,   5.688f},
    {  -2.808f,   5.688f},
    {  -2.466f,   5.746f},
    {  -2.173f,   6.039f},
    {  -1.929f,   6.567f},
    {  -1.929f,   7.154f},
    {  -2.076f,   7.681f},
    {  -2.418f,   8.385f},
    {  -2.271f,   9.088f},
    {  -2.027f,   9.382f},
    {  -1.734f,   9.851f},
    {  -1.490f,  10.261f},
    {  -1.343f,  10.906f},
    {  -1.343f,  11.375f},
    {  -1.490f,  11.844f},
    {  -1.880f,  12.196f},
    {  -2.076f,  12.548f},
    {  -3.004f,  12.431f},
    {  -3.492f,  12.313f},
    {  -3.736f,  12.020f},
    {  -3.834f,  11.844f},
    {  -4.078f,  11.903f},
    {  -4.127f,  12.196f},
    {  -3.883f,  12.607f},
    {  -3.590f,  12.724f},
    {  -3.248f,  12.958f},
    {  -2.613f,  12.900f},
    {  -2.173f,  12.900f},
    {  -1.783f,  12.783f},
    {  -1.587f,  12.607f},
    {  -1.392f,  12.431f},
    {  -1.197f,  12.255f},
    {  -1.001f,  11.962f},
    {  -0.855f,  11.551f},
    {  -0.757f,  11.141f},
    {  -0.757f,  10.672f},
    {  -0.611f,  10.378f},
    {  -0.562f,  10.027f},
    {  -0.659f,   9.675f},
    {  -1.001f,   9.558f},
    {  -1.245f,   9.382f},
    {  -1.538f,   9.206f},
    {  -1.734f,   8.854f},
    {  -1.832f,   8.561f},
    {  -1.783f,   8.209f},
    {  -1.636f,   7.857f},
    {  -1.490f,   7.623f},
    {  -1.441f,   7.329f},
    {  -1.392f,   6.860f},
    {  -1.392f,   6.509f},
    {  -1.538f,   6.274f},
    {  -1.636f,   5.922f},
    {  -1.783f,   5.629f},
    {  -2.076f,   5.336f},
    {  -2.369f,   5.160f},
    {  -2.857f,   5.160f},
    {  -3.248f,   5.277f},
    {  -3.736f,   5.160f},
    {  -4.225f,   4.925f},
    {  -4.518f,   4.749f},
    {  -5.104f,   4.398f},
    {  -5.543f,   4.104f},
    {  -5.983f,   3.870f},
    {  -6.374f,   3.753f},
    {  -6.618f,   3.577f},
    {  -6.911f,   3.518f},
    {  -7.155f,   3.401f},
    {  -7.643f,   3.342f},
    {  -8.083f,   3.401f},
    {  -8.327f,   3.459f},
    {  -8.669f,   3.694f},
    {  -8.864f,   3.929f},
    {  -9.158f,   4.104f},
};
static const int hole11_turfWall_seg1Count = 88;

// 'wall 2' (40 vertices)
static const float hole11_turfWall_seg2[][2] = {
    {  -8.132f,   6.039f},
    {  -7.741f,   5.981f},
    {  -7.595f,   6.274f},
    {  -7.399f,   6.450f},
    {  -7.253f,   6.567f},
    {  -6.911f,   6.567f},
    {  -6.569f,   6.684f},
    {  -6.471f,   6.978f},
    {  -6.325f,   7.329f},
    {  -6.374f,   7.916f},
    {  -6.276f,   8.385f},
    {  -5.983f,   8.619f},
    {  -5.788f,   8.971f},
    {  -5.690f,   9.147f},
    {  -5.446f,   9.499f},
    {  -5.201f,   9.733f},
    {  -5.006f,  10.027f},
    {  -4.908f,  10.261f},
    {  -4.762f,  10.437f},
    {  -4.762f,  10.730f},
    {  -5.104f,  10.848f},
    {  -5.299f,  10.613f},
    {  -5.397f,  10.378f},
    {  -5.543f,  10.144f},
    {  -5.641f,   9.968f},
    {  -5.739f,   9.792f},
    {  -5.885f,   9.616f},
    {  -6.032f,   9.440f},
    {  -6.129f,   9.206f},
    {  -6.374f,   8.971f},
    {  -6.569f,   8.678f},
    {  -6.764f,   8.385f},
    {  -6.960f,   8.092f},
    {  -7.155f,   7.799f},
    {  -7.350f,   7.564f},
    {  -7.546f,   7.271f},
    {  -7.692f,   7.095f},
    {  -7.790f,   6.919f},
    {  -7.985f,   6.626f},
    {  -8.132f,   6.391f},
};
static const int hole11_turfWall_seg2Count = 40;

static void addHole11TurfWallSegments(TurfWall* wall)
{
    wall->addSegment(hole11_turfWall_seg1, hole11_turfWall_seg1Count);
    wall->addSegment(hole11_turfWall_seg2, hole11_turfWall_seg2Count);
}
// END HOLE11_TURF_WALL_DATA

// BEGIN HOLE01_HOLE09_TURF_WALL_DATA (auto-generated — do not edit by hand)
// Source: hole-1-turf-wall.csv (holes 1 & 9 — draw once via createHole01Hole09)
// px 1..820, py 33..971 → 40×55 m map

// 'hole 9 turf wall' (30 vertices)
static const float hole0109_turfWall_seg1[][2] = {
    {  -4.567f, -16.652f},
    {  -5.543f, -16.183f},
    {  -6.471f, -15.714f},
    {  -7.106f, -16.594f},
    {  -7.643f, -17.649f},
    {  -7.839f, -19.057f},
    {  -7.595f, -19.877f},
    {  -7.253f, -20.640f},
    {  -7.204f, -20.933f},
    {  -8.034f, -20.991f},
    {  -8.864f, -21.402f},
    {  -9.304f, -21.930f},
    {  -9.255f, -22.340f},
    {  -8.718f, -22.340f},
    {  -8.083f, -21.519f},
    {  -7.350f, -21.519f},
    {  -6.569f, -21.636f},
    {  -5.836f, -21.578f},
    {  -5.934f, -20.933f},
    {  -6.276f, -20.581f},
    {  -6.618f, -20.405f},
    {  -6.960f, -20.053f},
    {  -7.155f, -19.467f},
    {  -7.302f, -18.705f},
    {  -7.155f, -18.177f},
    {  -6.960f, -17.591f},
    {  -6.471f, -17.004f},
    {  -5.885f, -16.887f},
    {  -5.153f, -17.122f},
    {  -4.518f, -17.239f},
};
static const int hole0109_turfWall_seg1Count = 30;

// 'wall 2' (28 vertices)
static const float hole0109_turfWall_seg2[][2] = {
    {  -3.101f, -17.532f},
    {  -1.685f, -17.825f},
    {  -1.099f, -18.001f},
    {  -0.513f, -18.060f},
    {   0.366f, -17.884f},
    {   1.001f, -17.708f},
    {   1.587f, -17.473f},
    {   1.978f, -17.415f},
    {   2.027f, -17.825f},
    {   1.538f, -18.118f},
    {   1.001f, -18.353f},
    {   0.464f, -18.646f},
    {   0.024f, -18.822f},
    {  -0.073f, -19.232f},
    {   0.317f, -19.467f},
    {   0.659f, -19.350f},
    {   1.148f, -19.057f},
    {   1.636f, -18.763f},
    {   1.734f, -19.467f},
    {   1.343f, -19.291f},
    {   1.050f, -19.643f},
    {   0.659f, -19.701f},
    {  -0.024f, -19.701f},
    {  -0.562f, -19.232f},
    {  -0.904f, -18.822f},
    {  -1.392f, -18.412f},
    {  -1.929f, -18.177f},
    {  -2.515f, -17.942f},
};
static const int hole0109_turfWall_seg2Count = 28;

// 'wall 3' (60 vertices)
static const float hole0109_turfWall_seg3[][2] = {
    {   3.394f, -16.594f},
    {   3.785f, -16.359f},
    {   4.225f, -16.594f},
    {   4.225f, -17.063f},
    {   4.420f, -17.591f},
    {   4.518f, -18.353f},
    {   4.225f, -19.115f},
    {   3.932f, -19.526f},
    {   3.932f, -19.936f},
    {   4.274f, -20.053f},
    {   4.860f, -20.464f},
    {   5.153f, -20.991f},
    {   5.201f, -21.754f},
    {   5.055f, -22.281f},
    {   4.762f, -22.868f},
    {   4.322f, -23.571f},
    {   3.883f, -24.216f},
    {   3.394f, -24.568f},
    {   2.808f, -24.861f},
    {   2.369f, -24.861f},
    {   2.320f, -24.334f},
    {   2.662f, -24.334f},
    {   3.150f, -24.099f},
    {   3.590f, -23.689f},
    {   3.883f, -23.220f},
    {   4.225f, -22.809f},
    {   4.518f, -22.223f},
    {   4.713f, -21.578f},
    {   4.615f, -20.933f},
    {   4.176f, -20.640f},
    {   3.687f, -20.405f},
    {   3.053f, -20.522f},
    {   2.613f, -20.698f},
    {   2.125f, -20.757f},
    {   1.343f, -20.816f},
    {   0.659f, -21.167f},
    {  -0.024f, -21.402f},
    {  -0.220f, -21.050f},
    {  -0.757f, -20.933f},
    {  -0.855f, -20.698f},
    {  -1.050f, -20.346f},
    {  -1.392f, -19.995f},
    {  -1.734f, -19.877f},
    {  -2.662f, -19.819f},
    {  -2.320f, -19.701f},
    {  -2.125f, -19.232f},
    {  -1.783f, -19.291f},
    {  -1.294f, -19.701f},
    {  -0.757f, -20.229f},
    {  -0.366f, -20.464f},
    {   0.073f, -20.581f},
    {   0.708f, -20.581f},
    {   1.441f, -20.171f},
    {   2.125f, -19.936f},
    {   2.711f, -19.701f},
    {   3.346f, -19.291f},
    {   3.883f, -18.763f},
    {   4.029f, -18.177f},
    {   3.932f, -17.532f},
    {   3.492f, -17.180f},
};
static const int hole0109_turfWall_seg3Count = 60;

// 'wall 4' (22 vertices)
static const float hole0109_turfWall_seg4[][2] = {
    {  -9.304f, -24.334f},
    {  -9.206f, -24.861f},
    {  -7.985f, -24.920f},
    {  -6.422f, -24.920f},
    {  -5.055f, -24.920f},
    {  -3.932f, -24.920f},
    {  -2.662f, -24.861f},
    {  -1.783f, -24.861f},
    {  -0.904f, -24.920f},
    {  -0.317f, -24.920f},
    {   0.220f, -24.861f},
    {   0.269f, -24.334f},
    {  -0.366f, -24.392f},
    {  -1.294f, -24.334f},
    {  -1.978f, -24.334f},
    {  -2.759f, -24.334f},
    {  -3.932f, -24.392f},
    {  -4.811f, -24.392f},
    {  -5.788f, -24.392f},
    {  -6.374f, -24.334f},
    {  -7.155f, -24.334f},
    {  -8.083f, -24.334f},
};
static const int hole0109_turfWall_seg4Count = 22;

static void addHole0109TurfWallSegments(TurfWall* wall)
{
    wall->addSegment(hole0109_turfWall_seg1, hole0109_turfWall_seg1Count);
    wall->addSegment(hole0109_turfWall_seg2, hole0109_turfWall_seg2Count);
    wall->addSegment(hole0109_turfWall_seg3, hole0109_turfWall_seg3Count);
    wall->addSegment(hole0109_turfWall_seg4, hole0109_turfWall_seg4Count);
}
// END HOLE01_HOLE09_TURF_WALL_DATA

// BEGIN HOLE03_TURF_WALL_DATA (auto-generated — do not edit by hand)
// Source: hole-3-turf-wall.csv
// px 1..820, py 33..971 → 40×55 m map

// 'hole 3 turf wall' (84 vertices)
static const float hole03_turfWall_seg1[][2] = {
    {  11.306f, -23.454f},
    {  11.697f, -23.513f},
    {  11.697f, -24.334f},
    {  11.746f, -24.979f},
    {  11.893f, -25.624f},
    {  12.332f, -25.800f},
    {  13.260f, -25.858f},
    {  14.090f, -25.858f},
    {  14.969f, -25.858f},
    {  15.751f, -25.800f},
    {  16.532f, -25.800f},
    {  16.825f, -25.800f},
    {  17.167f, -25.800f},
    {  17.753f, -25.741f},
    {  18.095f, -25.741f},
    {  18.388f, -25.565f},
    {  18.779f, -25.096f},
    {  19.121f, -24.744f},
    {  19.414f, -24.392f},
    {  19.512f, -24.041f},
    {  19.560f, -23.220f},
    {  19.560f, -22.281f},
    {  19.560f, -21.402f},
    {  19.609f, -20.816f},
    {  19.609f, -20.229f},
    {  19.609f, -19.584f},
    {  19.560f, -19.115f},
    {  19.560f, -18.646f},
    {  19.560f, -18.060f},
    {  19.609f, -17.473f},
    {  19.512f, -17.122f},
    {  19.267f, -16.652f},
    {  18.974f, -16.418f},
    {  18.535f, -16.242f},
    {  18.095f, -16.242f},
    {  17.753f, -16.594f},
    {  17.460f, -17.122f},
    {  17.314f, -17.649f},
    {  17.314f, -18.118f},
    {  17.460f, -18.470f},
    {  17.656f, -19.115f},
    {  17.509f, -19.584f},
    {  17.265f, -19.291f},
    {  17.070f, -18.646f},
    {  16.972f, -18.001f},
    {  16.972f, -17.356f},
    {  17.021f, -16.828f},
    {  17.314f, -16.242f},
    {  17.607f, -15.949f},
    {  18.144f, -15.656f},
    {  18.632f, -15.714f},
    {  19.023f, -15.832f},
    {  19.316f, -16.066f},
    {  19.756f, -16.477f},
    {  19.951f, -17.122f},
    {  20.000f, -17.649f},
    {  20.000f, -18.353f},
    {  19.951f, -19.115f},
    {  20.000f, -19.760f},
    {  20.000f, -20.581f},
    {  20.000f, -21.109f},
    {  20.000f, -22.106f},
    {  19.951f, -22.926f},
    {  19.951f, -23.630f},
    {  20.000f, -24.275f},
    {  19.609f, -24.803f},
    {  19.365f, -25.272f},
    {  18.974f, -25.624f},
    {  18.681f, -26.034f},
    {  18.291f, -26.210f},
    {  17.656f, -26.327f},
    {  16.484f, -26.386f},
    {  15.604f, -26.386f},
    {  14.823f, -26.327f},
    {  14.042f, -26.386f},
    {  13.260f, -26.327f},
    {  12.625f, -26.327f},
    {  11.941f, -26.210f},
    {  11.697f, -26.034f},
    {  11.306f, -25.624f},
    {  11.258f, -25.272f},
    {  11.258f, -24.979f},
    {  11.258f, -24.627f},
    {  11.258f, -24.041f},
};
static const int hole03_turfWall_seg1Count = 84;

// 'wall 2' (27 vertices)
static const float hole03_turfWall_seg2[][2] = {
    {  13.358f, -23.513f},
    {  13.407f, -24.041f},
    {  14.188f, -24.099f},
    {  14.921f, -24.041f},
    {  15.653f, -24.099f},
    {  16.093f, -24.158f},
    {  16.581f, -24.099f},
    {  17.167f, -24.041f},
    {  17.656f, -23.689f},
    {  17.949f, -22.985f},
    {  17.949f, -22.223f},
    {  17.949f, -21.402f},
    {  17.851f, -20.581f},
    {  17.802f, -20.229f},
    {  17.558f, -20.757f},
    {  17.509f, -21.109f},
    {  17.460f, -21.461f},
    {  17.314f, -21.930f},
    {  17.070f, -22.281f},
    {  16.825f, -22.633f},
    {  16.435f, -22.985f},
    {  16.093f, -23.278f},
    {  15.751f, -23.513f},
    {  15.165f, -23.630f},
    {  14.530f, -23.571f},
    {  14.090f, -23.571f},
    {  13.748f, -23.513f},
};
static const int hole03_turfWall_seg2Count = 27;

static void addHole03TurfWallSegments(TurfWall* wall)
{
    wall->addSegment(hole03_turfWall_seg1, hole03_turfWall_seg1Count);
    wall->addSegment(hole03_turfWall_seg2, hole03_turfWall_seg2Count);
}
// END HOLE03_TURF_WALL_DATA

// BEGIN HOLE17_TURF_WALL_DATA (auto-generated — do not edit by hand)
// Source: hole-17-turf-wall.csv

// 'hole 17 turf wall' (27 vertices)
static const float hole17_turfWall_seg1[][2] = {
    { -19.805f,   5.336f},
    { -19.463f,   5.336f},
    { -19.267f,   4.515f},
    { -19.023f,   4.046f},
    { -19.023f,   3.401f},
    { -19.023f,   2.639f},
    { -19.365f,   2.287f},
    { -19.463f,   1.466f},
    { -19.219f,   0.762f},
    { -18.926f,   0.528f},
    { -18.291f,   0.293f},
    { -17.656f,   0.293f},
    { -16.923f,   0.352f},
    { -16.288f,   1.114f},
    { -15.946f,   0.704f},
    { -16.435f,   0.235f},
    { -16.825f,  -0.059f},
    { -17.411f,  -0.293f},
    { -18.144f,  -0.352f},
    { -18.730f,  -0.235f},
    { -19.267f,   0.293f},
    { -19.658f,   0.880f},
    { -19.805f,   1.349f},
    { -19.853f,   2.404f},
    { -19.853f,   2.932f},
    { -19.805f,   4.280f},
    { -19.805f,   4.808f},
};
static const int hole17_turfWall_seg1Count = 27;

// 'wall 2' (10 vertices)
static const float hole17_turfWall_seg2[][2] = {
    { -16.679f,   5.864f},
    { -16.337f,   5.981f},
    { -16.044f,   5.160f},
    { -15.849f,   4.456f},
    { -15.800f,   3.811f},
    { -15.800f,   3.108f},
    { -16.142f,   3.225f},
    { -16.142f,   4.104f},
    { -16.288f,   4.749f},
    { -16.532f,   5.512f},
};
static const int hole17_turfWall_seg2Count = 10;

// 'wall 3' (19 vertices)
static const float hole17_turfWall_seg3[][2] = {
    { -19.609f,   9.147f},
    { -19.072f,   9.147f},
    { -19.121f,   9.909f},
    { -19.072f,  10.906f},
    { -18.828f,  11.551f},
    { -18.437f,  11.727f},
    { -17.900f,  11.727f},
    { -17.509f,  11.551f},
    { -17.265f,  11.141f},
    { -16.874f,  11.023f},
    { -16.777f,  11.493f},
    { -17.265f,  11.962f},
    { -17.900f,  12.372f},
    { -18.584f,  12.431f},
    { -19.023f,  12.138f},
    { -19.365f,  11.668f},
    { -19.512f,  11.141f},
    { -19.560f,  10.672f},
    { -19.658f,   9.851f},
};
static const int hole17_turfWall_seg3Count = 19;

static void addHole17TurfWallSegments(TurfWall* wall)
{
    wall->addSegment(hole17_turfWall_seg1, hole17_turfWall_seg1Count);
    wall->addSegment(hole17_turfWall_seg2, hole17_turfWall_seg2Count);
    wall->addSegment(hole17_turfWall_seg3, hole17_turfWall_seg3Count);
}
// END HOLE17_TURF_WALL_DATA

// BEGIN HOLE18_TURF_WALL_DATA (auto-generated — do not edit by hand)
// Source: hole-18-turf-wall.csv
// px 1..820, py 33..971 → 40×55 m map

// 'hole 18 turf wall' (33 vertices)
static const float hole18_turfWall_seg1[][2] = {
    { -18.779f, -13.838f},
    { -18.584f, -13.779f},
    { -18.486f, -14.717f},
    { -18.339f, -15.187f},
    { -17.851f, -15.362f},
    { -17.460f, -15.304f},
    { -17.265f, -15.597f},
    { -17.070f, -15.714f},
    { -16.630f, -15.773f},
    { -16.288f, -15.538f},
    { -16.044f, -15.187f},
    { -15.800f, -14.835f},
    { -15.751f, -14.248f},
    { -15.604f, -13.662f},
    { -15.556f, -13.076f},
    { -15.507f, -12.313f},
    { -15.360f, -12.079f},
    { -15.067f, -12.079f},
    { -15.165f, -13.076f},
    { -15.214f, -14.072f},
    { -15.311f, -14.835f},
    { -15.507f, -15.304f},
    { -15.751f, -15.656f},
    { -16.190f, -16.066f},
    { -16.581f, -16.301f},
    { -16.972f, -16.359f},
    { -17.607f, -16.359f},
    { -18.095f, -16.007f},
    { -18.388f, -15.714f},
    { -18.632f, -15.421f},
    { -18.730f, -15.069f},
    { -18.828f, -14.717f},
    { -18.828f, -14.307f},
};
static const int hole18_turfWall_seg1Count = 33;

// 'wall 2' (25 vertices)
static const float hole18_turfWall_seg2[][2] = {
    { -15.018f, -10.554f},
    { -14.676f, -10.496f},
    { -14.676f,  -9.968f},
    { -14.676f,  -9.440f},
    { -14.725f,  -8.619f},
    { -14.823f,  -7.857f},
    { -14.969f,  -7.329f},
    { -15.311f,  -6.391f},
    { -15.653f,  -5.512f},
    { -15.800f,  -5.043f},
    { -16.190f,  -4.925f},
    { -16.190f,  -5.570f},
    { -16.044f,  -6.098f},
    { -15.946f,  -6.626f},
    { -15.849f,  -6.919f},
    { -15.702f,  -7.095f},
    { -15.507f,  -7.505f},
    { -15.263f,  -7.916f},
    { -15.067f,  -8.209f},
    { -15.067f,  -8.502f},
    { -15.067f,  -8.795f},
    { -15.067f,  -9.088f},
    { -15.067f,  -9.382f},
    { -15.067f,  -9.616f},
    { -15.116f, -10.085f},
};
static const int hole18_turfWall_seg2Count = 25;

// 'wall 3' (23 vertices)
static const float hole18_turfWall_seg3[][2] = {
    { -18.730f,  -4.925f},
    { -18.486f,  -4.984f},
    { -18.437f,  -4.456f},
    { -18.437f,  -3.929f},
    { -18.388f,  -3.694f},
    { -18.242f,  -3.401f},
    { -18.095f,  -3.225f},
    { -17.900f,  -3.166f},
    { -17.558f,  -3.049f},
    { -17.216f,  -3.049f},
    { -16.972f,  -3.284f},
    { -16.728f,  -3.342f},
    { -16.728f,  -2.756f},
    { -17.070f,  -2.639f},
    { -17.314f,  -2.639f},
    { -17.656f,  -2.639f},
    { -17.998f,  -2.639f},
    { -18.242f,  -2.756f},
    { -18.437f,  -3.049f},
    { -18.681f,  -3.342f},
    { -18.779f,  -3.694f},
    { -18.828f,  -4.163f},
    { -18.828f,  -4.515f},
};
static const int hole18_turfWall_seg3Count = 23;

static void addHole18TurfWallSegments(TurfWall* wall)
{
    wall->addSegment(hole18_turfWall_seg1, hole18_turfWall_seg1Count);
    wall->addSegment(hole18_turfWall_seg2, hole18_turfWall_seg2Count);
    wall->addSegment(hole18_turfWall_seg3, hole18_turfWall_seg3Count);
}
// END HOLE18_TURF_WALL_DATA

// BEGIN HOLE05_TURF_WALL_DATA (auto-generated — do not edit by hand)
// Source: hole-5-turf-wall.csv
// px 1..820, py 33..971 → 40×55 m map

// 'hole 5 turf wall' (21 vertices)
static const float hole05_turfWall_seg1[][2] = {
    {  14.335f,  -5.570f},
    {  14.725f,  -5.864f},
    {  15.067f,  -5.629f},
    {  15.507f,  -5.219f},
    {  15.653f,  -4.339f},
    {  15.751f,  -3.342f},
    {  15.751f,  -1.818f},
    {  15.702f,  -0.938f},
    {  15.702f,   0.704f},
    {  15.702f,   1.759f},
    {  15.653f,   2.404f},
    {  15.458f,   2.697f},
    {  15.311f,   2.580f},
    {  15.311f,   1.583f},
    {  15.311f,  -0.000f},
    {  15.311f,  -1.349f},
    {  15.360f,  -2.697f},
    {  15.311f,  -3.284f},
    {  15.165f,  -3.694f},
    {  15.214f,  -4.398f},
    {  14.725f,  -5.043f},
};
static const int hole05_turfWall_seg1Count = 21;

// 'wall 2' (86 vertices)
static const float hole05_turfWall_seg2[][2] = {
    {  15.995f,  -8.619f},
    {  15.995f,  -8.971f},
    {  16.288f,  -8.971f},
    {  16.728f,  -8.678f},
    {  16.972f,  -8.268f},
    {  17.070f,  -7.974f},
    {  17.167f,  -6.743f},
    {  17.167f,  -5.864f},
    {  17.314f,  -5.160f},
    {  17.509f,  -4.574f},
    {  17.509f,  -4.104f},
    {  17.558f,  -3.518f},
    {  17.802f,  -3.284f},
    {  18.242f,  -2.639f},
    {  18.486f,  -1.994f},
    {  18.730f,  -1.525f},
    {  19.023f,  -0.997f},
    {  19.365f,  -0.469f},
    {  19.658f,   0.469f},
    {  19.805f,   0.880f},
    {  19.805f,   1.466f},
    {  19.805f,   1.876f},
    {  19.658f,   2.345f},
    {  19.609f,   2.756f},
    {  19.463f,   3.049f},
    {  19.267f,   3.518f},
    {  19.072f,   4.398f},
    {  19.023f,   4.691f},
    {  18.877f,   5.336f},
    {  18.730f,   5.746f},
    {  18.584f,   6.215f},
    {  18.046f,   6.860f},
    {  17.509f,   7.154f},
    {  16.874f,   7.154f},
    {  16.337f,   7.095f},
    {  15.995f,   6.860f},
    {  15.556f,   6.274f},
    {  15.360f,   5.746f},
    {  15.263f,   5.160f},
    {  15.214f,   4.632f},
    {  15.360f,   4.456f},
    {  15.604f,   4.632f},
    {  15.702f,   5.101f},
    {  15.800f,   5.629f},
    {  15.995f,   6.039f},
    {  16.190f,   6.391f},
    {  16.484f,   6.567f},
    {  16.874f,   6.743f},
    {  17.411f,   6.684f},
    {  17.607f,   6.333f},
    {  17.753f,   5.981f},
    {  17.900f,   5.688f},
    {  17.998f,   5.453f},
    {  18.193f,   5.394f},
    {  18.339f,   5.219f},
    {  18.437f,   4.984f},
    {  18.535f,   4.808f},
    {  18.632f,   4.515f},
    {  18.681f,   4.222f},
    {  18.779f,   3.811f},
    {  18.926f,   3.342f},
    {  19.023f,   2.932f},
    {  19.219f,   2.521f},
    {  19.316f,   2.170f},
    {  19.365f,   1.759f},
    {  19.365f,   0.997f},
    {  19.365f,   0.704f},
    {  19.267f,   0.469f},
    {  18.974f,  -0.176f},
    {  18.730f,  -0.645f},
    {  18.584f,  -0.938f},
    {  18.339f,  -1.407f},
    {  18.144f,  -1.700f},
    {  17.998f,  -2.052f},
    {  17.851f,  -2.345f},
    {  17.705f,  -2.580f},
    {  17.558f,  -2.873f},
    {  17.314f,  -3.049f},
    {  17.216f,  -3.929f},
    {  17.070f,  -4.515f},
    {  16.825f,  -5.219f},
    {  16.679f,  -5.746f},
    {  16.581f,  -6.039f},
    {  16.581f,  -7.974f},
    {  16.435f,  -8.268f},
    {  16.288f,  -8.502f},
};
static const int hole05_turfWall_seg2Count = 86;

static void addHole05TurfWallSegments(TurfWall* wall)
{
    wall->addSegment(hole05_turfWall_seg1, hole05_turfWall_seg1Count);
    wall->addSegment(hole05_turfWall_seg2, hole05_turfWall_seg2Count);
}
// END HOLE05_TURF_WALL_DATA

// BEGIN HOLE10_TURF_WALL_DATA (auto-generated — do not edit by hand)
// Source: hole-10-turf-wall.csv
// px 1..820, py 33..971 → 40×55 m map

// 'hole 10 turf wall' (32 vertices)
static const float hole10_turfWall_seg1[][2] = {
    { -11.404f,   1.173f},
    { -11.160f,   0.704f},
    { -11.502f,   0.059f},
    { -11.746f,  -0.586f},
    { -12.088f,  -1.290f},
    { -12.430f,  -2.345f},
    { -12.674f,  -3.225f},
    { -12.821f,  -4.163f},
    { -12.772f,  -6.391f},
    { -12.527f,  -7.154f},
    { -12.137f,  -7.974f},
    { -11.648f,  -9.030f},
    { -10.965f,  -9.968f},
    { -10.330f, -10.378f},
    { -10.379f, -10.906f},
    { -10.867f, -10.672f},
    { -11.306f, -10.261f},
    { -11.648f,  -9.792f},
    { -12.283f,  -8.854f},
    { -12.821f,  -7.681f},
    { -13.016f,  -6.919f},
    { -13.114f,  -6.509f},
    { -13.211f,  -5.688f},
    { -13.260f,  -4.398f},
    { -13.211f,  -3.635f},
    { -13.065f,  -3.108f},
    { -12.967f,  -2.521f},
    { -12.821f,  -1.700f},
    { -12.527f,  -1.114f},
    { -12.332f,  -0.469f},
    { -11.990f,   0.176f},
    { -11.746f,   0.704f},
};
static const int hole10_turfWall_seg1Count = 32;

// 'wall 2' (43 vertices)
static const float hole10_turfWall_seg2[][2] = {
    { -10.085f,   3.284f},
    {  -9.304f,   2.756f},
    {  -8.571f,   2.287f},
    {  -8.181f,   1.935f},
    {  -7.741f,   1.290f},
    {  -7.350f,   0.645f},
    {  -7.302f,  -0.410f},
    {  -7.302f,  -1.055f},
    {  -7.595f,  -1.700f},
    {  -7.790f,  -2.345f},
    {  -8.034f,  -3.049f},
    {  -8.327f,  -3.870f},
    {  -8.571f,  -4.632f},
    {  -8.718f,  -5.394f},
    {  -8.816f,  -6.391f},
    {  -8.913f,  -7.271f},
    {  -8.864f,  -8.033f},
    {  -8.816f,  -8.561f},
    {  -8.620f,  -8.913f},
    {  -8.669f,  -9.440f},
    {  -9.011f,  -9.440f},
    {  -9.255f,  -9.206f},
    {  -9.255f,  -8.502f},
    {  -9.255f,  -7.681f},
    {  -9.206f,  -6.684f},
    {  -9.206f,  -5.922f},
    {  -9.109f,  -5.160f},
    {  -9.011f,  -4.632f},
    {  -8.864f,  -4.163f},
    {  -8.669f,  -3.577f},
    {  -8.474f,  -2.990f},
    {  -8.181f,  -2.228f},
    {  -7.937f,  -1.525f},
    {  -7.790f,  -0.821f},
    {  -7.692f,   0.059f},
    {  -7.888f,   0.469f},
    {  -8.181f,   0.997f},
    {  -8.816f,   1.407f},
    {  -9.255f,   1.759f},
    {  -9.744f,   1.994f},
    { -10.134f,   2.463f},
    { -10.330f,   2.697f},
    { -10.281f,   3.049f},
};
static const int hole10_turfWall_seg2Count = 43;

static void addHole10TurfWallSegments(TurfWall* wall)
{
    wall->addSegment(hole10_turfWall_seg1, hole10_turfWall_seg1Count);
    wall->addSegment(hole10_turfWall_seg2, hole10_turfWall_seg2Count);
}
// END HOLE10_TURF_WALL_DATA

// BEGIN HOLE08_TURF_WALL_DATA (auto-generated — do not edit by hand)
// Source: hole-8-turf-wall.csv
// px 1..820, py 33..971 → 40×55 m map

// 'hole 8 turf wall' (50 vertices)
static const float hole08_turfWall_seg1[][2] = {
    {  -0.415f, -17.004f},
    {  -1.392f, -16.887f},
    {  -2.320f, -16.301f},
    {  -2.808f, -15.773f},
    {  -2.808f, -14.776f},
    {  -2.711f, -14.014f},
    {  -1.978f, -13.134f},
    {  -1.392f, -12.724f},
    {  -0.806f, -12.372f},
    {  -0.464f, -12.079f},
    {   0.024f, -11.786f},
    {   0.366f, -11.493f},
    {   0.757f, -11.199f},
    {   1.099f, -11.023f},
    {   1.490f, -10.789f},
    {   2.027f, -10.320f},
    {   2.564f,  -9.909f},
    {   3.053f,  -9.675f},
    {   3.687f,  -9.206f},
    {   4.176f,  -8.795f},
    {   4.615f,  -8.737f},
    {   5.006f,  -8.443f},
    {   5.592f,  -8.443f},
    {   6.325f,  -8.443f},
    {   7.204f,  -8.443f},
    {   7.350f,  -8.795f},
    {   7.106f,  -8.854f},
    {   6.618f,  -8.854f},
    {   6.081f,  -8.913f},
    {   5.299f,  -8.971f},
    {   4.811f,  -9.206f},
    {   4.225f,  -9.382f},
    {   3.834f,  -9.733f},
    {   3.443f,  -9.909f},
    {   3.101f, -10.203f},
    {   2.564f, -10.613f},
    {   2.027f, -11.023f},
    {   1.343f, -11.434f},
    {   0.562f, -12.020f},
    {   0.024f, -12.372f},
    {  -0.904f, -12.958f},
    {  -1.392f, -13.252f},
    {  -1.783f, -13.603f},
    {  -2.222f, -14.072f},
    {  -2.369f, -14.600f},
    {  -2.271f, -15.421f},
    {  -2.076f, -15.832f},
    {  -1.538f, -16.242f},
    {  -0.855f, -16.359f},
    {  -0.464f, -16.477f},
};
static const int hole08_turfWall_seg1Count = 50;

// 'turf wall 2' (52 vertices)
static const float hole08_turfWall_seg2[][2] = {
    {   0.952f, -16.007f},
    {   1.050f, -16.359f},
    {   1.245f, -16.359f},
    {   1.636f, -15.832f},
    {   1.880f, -15.362f},
    {   2.027f, -15.069f},
    {   2.076f, -14.659f},
    {   2.222f, -14.483f},
    {   2.418f, -13.955f},
    {   2.564f, -13.721f},
    {   2.857f, -13.369f},
    {   3.053f, -13.076f},
    {   3.297f, -12.783f},
    {   3.687f, -12.431f},
    {   4.029f, -12.255f},
    {   4.274f, -12.138f},
    {   4.615f, -12.020f},
    {   4.860f, -11.903f},
    {   5.006f, -11.844f},
    {   6.667f, -11.844f},
    {   6.960f, -12.020f},
    {   7.155f, -12.020f},
    {   7.643f, -12.020f},
    {   7.790f, -11.844f},
    {   7.985f, -11.668f},
    {   7.937f, -11.258f},
    {   7.204f, -11.199f},
    {   6.960f, -11.317f},
    {   5.495f, -11.375f},
    {   5.104f, -11.434f},
    {   4.567f, -11.375f},
    {   4.567f, -11.610f},
    {   4.176f, -11.610f},
    {   4.078f, -11.786f},
    {   3.785f, -11.844f},
    {   3.736f, -12.020f},
    {   3.492f, -12.079f},
    {   3.297f, -12.313f},
    {   3.101f, -12.372f},
    {   3.004f, -12.607f},
    {   2.857f, -12.665f},
    {   2.662f, -12.900f},
    {   2.515f, -13.076f},
    {   2.418f, -13.252f},
    {   2.173f, -13.428f},
    {   1.978f, -13.897f},
    {   1.783f, -14.483f},
    {   1.685f, -15.011f},
    {   1.538f, -15.187f},
    {   1.441f, -15.421f},
    {   1.343f, -15.656f},
    {   1.099f, -15.832f},
};
static const int hole08_turfWall_seg2Count = 52;

static void addHole08TurfWallSegments(TurfWall* wall)
{
    wall->addSegment(hole08_turfWall_seg1, hole08_turfWall_seg1Count);
    wall->addSegment(hole08_turfWall_seg2, hole08_turfWall_seg2Count);
}
// END HOLE08_TURF_WALL_DATA

// BEGIN HOLE04_TURF_WALL_DATA (auto-generated — do not edit by hand)
// Source: hole-4-turf-wall.csv
// px 1..820, py 33..971 → 40×55 m map

// outer berm (107 vertices)
static const float hole04_turfWall_seg1[][2] = {
    {  13.065f,  -7.271f},
    {  13.309f,  -7.388f},
    {  13.407f,  -7.329f},
    {  13.455f,  -6.802f},
    {  13.455f,  -6.333f},
    {  13.358f,  -5.805f},
    {  13.260f,  -5.512f},
    {  13.162f,  -5.160f},
    {  12.821f,  -4.867f},
    {  12.527f,  -4.632f},
    {  12.039f,  -4.574f},
    {  11.697f,  -4.574f},
    {  11.404f,  -4.691f},
    {  10.769f,  -5.101f},
    {  10.134f,  -5.746f},
    {   9.988f,  -6.157f},
    {   9.792f,  -6.684f},
    {   9.597f,  -7.329f},
    {   9.548f,  -8.209f},
    {   9.548f,  -9.264f},
    {   9.548f,  -9.968f},
    {   9.597f, -10.378f},
    {   9.744f, -11.199f},
    {  10.183f, -11.903f},
    {  10.867f, -12.431f},
    {  11.697f, -13.017f},
    {  12.381f, -13.310f},
    {  13.504f, -13.662f},
    {  13.114f, -13.486f},
    {  14.188f, -13.779f},
    {  14.823f, -14.014f},
    {  15.360f, -14.131f},
    {  15.995f, -14.190f},
    {  16.777f, -14.190f},
    {  12.527f, -11.727f},
    {  12.430f, -11.082f},
    {  12.381f, -10.789f},
    {  12.430f, -10.320f},
    {  12.576f, -10.027f},
    {  12.967f,  -9.558f},
    {  13.407f,  -9.323f},
    {  13.797f,  -9.264f},
    {  14.090f,  -9.323f},
    {  14.481f,  -9.499f},
    {  14.676f,  -9.733f},
    {  14.921f,  -9.909f},
    {  14.969f, -10.144f},
    {  15.116f, -10.496f},
    {  15.116f, -10.906f},
    {  15.067f, -11.082f},
    {  14.921f, -11.493f},
    {  15.311f, -11.141f},
    {  15.507f, -11.023f},
    {  15.751f, -10.906f},
    {  16.142f, -10.906f},
    {  16.337f, -11.082f},
    {  16.581f, -11.317f},
    {  16.923f, -11.668f},
    {  17.216f, -11.844f},
    {  17.363f, -11.727f},
    {  17.411f, -11.434f},
    {  17.118f, -11.141f},
    {  16.532f, -10.672f},
    {  15.897f, -10.261f},
    {  15.263f,  -9.792f},
    {  14.725f,  -9.323f},
    {  13.846f,  -8.619f},
    {  13.553f,  -8.678f},
    {  13.260f,  -8.913f},
    {  12.967f,  -9.206f},
    {  12.821f,  -9.440f},
    {  12.723f,  -9.675f},
    {  12.430f,  -9.909f},
    {  11.990f,  -9.968f},
    {  11.746f, -10.085f},
    {  11.648f, -10.437f},
    {  11.697f, -10.730f},
    {  11.990f, -10.906f},
    {  12.234f, -11.082f},
    {  12.332f, -11.434f},
    {  12.137f, -11.493f},
    {  11.844f, -11.610f},
    {  11.453f, -11.668f},
    {  11.013f, -11.668f},
    {  10.672f, -11.610f},
    {  10.427f, -11.317f},
    {  10.183f, -11.023f},
    {  10.037f, -10.613f},
    {   9.988f, -10.027f},
    {   9.890f,  -9.382f},
    {   9.890f,  -8.737f},
    {   9.988f,  -7.681f},
    {  10.085f,  -7.095f},
    {  10.183f,  -6.743f},
    {  10.427f,  -6.274f},
    {  10.672f,  -5.864f},
    {  10.916f,  -5.570f},
    {  11.209f,  -5.219f},
    {  11.600f,  -5.160f},
    {  11.600f,  -4.925f},
    {  12.332f,  -4.925f},
    {  12.430f,  -5.043f},
    {  12.625f,  -5.101f},
    {  12.821f,  -5.336f},
    {  12.967f,  -5.805f},
    {  13.016f,  -6.274f},
    {  12.967f,  -6.743f},
};
static const int hole04_turfWall_seg1Count = 107;

// inner oval (16 vertices)
static const float hole04_turfWall_seg2[][2] = {
    {  17.411f, -14.190f},
    {  17.460f, -13.779f},
    {  16.923f, -13.779f},
    {  16.679f, -13.779f},
    {  16.044f, -13.721f},
    {  15.653f, -13.603f},
    {  15.311f, -13.545f},
    {  14.921f, -13.545f},
    {  14.725f, -13.545f},
    {  14.335f, -13.428f},
    {  13.895f, -13.310f},
    {  13.748f, -13.134f},
    {  13.358f, -12.958f},
    {  13.211f, -12.724f},
    {  12.821f, -12.431f},
    {  12.674f, -12.079f},
};
static const int hole04_turfWall_seg2Count = 16;

static void addHole04TurfWallSegments(TurfWall* wall)
{
    wall->addSegment(hole04_turfWall_seg1, hole04_turfWall_seg1Count);
    wall->addSegment(hole04_turfWall_seg2, hole04_turfWall_seg2Count);
}
// END HOLE04_TURF_WALL_DATA

// BEGIN HOLE02_TURF_WALL_DATA (auto-generated — do not edit by hand)
// Source: hole2-turf-walls.csv
// px 1..820, py 33..971 → 40×55 m map

// segment 1 (45 vertices)
static const float hole02_turfWall_seg1[][2] = {
    {   6.276f, -20.405f},
    {   6.618f, -20.053f},
    {   5.885f, -18.881f},
    {   6.081f, -18.118f},
    {   6.618f, -17.825f},
    {   7.790f, -16.477f},
    {   8.571f, -15.890f},
    {   9.255f, -15.538f},
    {   9.939f, -15.128f},
    {  10.623f, -15.069f},
    {  11.697f, -15.128f},
    {  12.674f, -15.538f},
    {  13.358f, -15.773f},
    {  13.944f, -16.477f},
    {  14.676f, -17.297f},
    {  15.263f, -18.236f},
    {  15.751f, -19.467f},
    {  16.044f, -20.288f},
    {  16.044f, -20.874f},
    {  15.897f, -21.461f},
    {  15.409f, -21.871f},
    {  14.628f, -21.930f},
    {  14.628f, -22.457f},
    {  15.409f, -22.399f},
    {  16.044f, -21.988f},
    {  16.532f, -21.226f},
    {  16.532f, -20.464f},
    {  16.288f, -19.526f},
    {  15.995f, -18.529f},
    {  15.263f, -17.180f},
    {  14.335f, -16.007f},
    {  13.065f, -15.069f},
    {  11.795f, -14.483f},
    {  11.209f, -14.424f},
    {  10.183f, -14.542f},
    {   9.499f, -14.717f},
    {   9.011f, -15.011f},
    {   8.327f, -15.362f},
    {   7.643f, -15.890f},
    {   7.155f, -16.477f},
    {   6.569f, -17.004f},
    {   6.032f, -17.473f},
    {   5.641f, -18.118f},
    {   5.446f, -18.939f},
    {   5.885f, -19.877f},
};
static const int hole02_turfWall_seg1Count = 45;

// segment 2 (21 vertices)
static const float hole02_turfWall_seg2[][2] = {
    {   8.718f, -16.477f},
    {   8.816f, -17.004f},
    {   9.451f, -16.652f},
    {  10.330f, -16.594f},
    {  11.160f, -16.594f},
    {  11.697f, -16.652f},
    {  12.381f, -17.004f},
    {  12.821f, -17.239f},
    {  13.553f, -17.884f},
    {  13.944f, -17.825f},
    {  13.700f, -17.356f},
    {  13.114f, -16.770f},
    {  12.479f, -16.066f},
    {  11.941f, -15.890f},
    {  11.404f, -15.832f},
    {  10.916f, -15.773f},
    {  10.330f, -15.832f},
    {   9.744f, -16.066f},
    {   9.206f, -16.242f},
    {   8.523f, -17.473f},
    {   8.523f, -18.001f},
};
static const int hole02_turfWall_seg2Count = 21;

// segment 3 (18 vertices)
static const float hole02_turfWall_seg3[][2] = {
    {   9.597f, -17.942f},
    {   9.988f, -18.294f},
    {  10.330f, -18.294f},
    {  11.062f, -18.881f},
    {  11.844f, -18.939f},
    {  12.723f, -18.998f},
    {  13.455f, -19.350f},
    {  13.846f, -19.526f},
    {  13.846f, -19.115f},
    {  13.260f, -18.353f},
    {  12.625f, -17.767f},
    {  11.746f, -17.297f},
    {  10.818f, -17.122f},
    {  10.134f, -17.122f},
    {   9.646f, -17.297f},
    {   8.864f, -17.473f},
    {   8.767f, -18.412f},
    {   8.816f, -18.998f},
};
static const int hole02_turfWall_seg3Count = 18;

// segment 4 (16 vertices)
static const float hole02_turfWall_seg4[][2] = {
    {   9.499f, -19.232f},
    {   9.988f, -19.584f},
    {  10.379f, -20.053f},
    {  10.720f, -20.288f},
    {  11.013f, -20.405f},
    {  11.502f, -20.405f},
    {  12.039f, -20.464f},
    {  12.674f, -20.522f},
    {  13.114f, -20.464f},
    {  13.065f, -20.112f},
    {  12.381f, -19.819f},
    {  11.551f, -19.760f},
    {  10.916f, -19.467f},
    {  10.281f, -19.057f},
    {   9.646f, -18.646f},
    {   9.109f, -18.587f},
};
static const int hole02_turfWall_seg4Count = 16;

// segment 5 (18 vertices)
static const float hole02_turfWall_seg5[][2] = {
    {   7.790f, -19.408f},
    {   7.790f, -19.760f},
    {   8.425f, -19.877f},
    {   9.158f, -20.464f},
    {   9.792f, -21.343f},
    {  10.330f, -21.695f},
    {  10.867f, -22.047f},
    {  11.502f, -22.164f},
    {  12.332f, -22.281f},
    {  12.967f, -22.223f},
    {  12.821f, -21.812f},
    {  12.234f, -21.754f},
    {  11.600f, -21.578f},
    {  10.867f, -21.167f},
    {  10.281f, -20.816f},
    {   9.646f, -20.112f},
    {   9.158f, -19.760f},
    {   8.571f, -19.467f},
};
static const int hole02_turfWall_seg5Count = 18;

static void addHole02TurfWallSegments(TurfWall* wall)
{
    wall->addSegment(hole02_turfWall_seg1, hole02_turfWall_seg1Count);
    wall->addSegment(hole02_turfWall_seg2, hole02_turfWall_seg2Count);
    wall->addSegment(hole02_turfWall_seg3, hole02_turfWall_seg3Count);
    wall->addSegment(hole02_turfWall_seg4, hole02_turfWall_seg4Count);
    wall->addSegment(hole02_turfWall_seg5, hole02_turfWall_seg5Count);
}
// END HOLE02_TURF_WALL_DATA

// BEGIN HOLE06_TURF_WALL_DATA (auto-generated — do not edit by hand)
// Source: hole-6-turf-wall.csv

// 'hole 8 turf wall' (17 vertices)
static const float hole06_turfWall_seg1[][2] = {
    {  10.818f,  18.118f},
    {  11.062f,  18.118f},
    {  11.111f,  17.415f},
    {  11.111f,  16.946f},
    {  11.062f,  16.242f},
    {  11.062f,  15.421f},
    {  11.160f,  14.776f},
    {  11.306f,  14.072f},
    {  11.453f,  13.369f},
    {  11.502f,  12.958f},
    {  11.209f,  13.134f},
    {  11.013f,  13.603f},
    {  10.867f,  14.248f},
    {  10.818f,  15.304f},
    {  10.769f,  16.066f},
    {  10.720f,  16.946f},
    {  10.720f,  17.473f},
};
static const int hole06_turfWall_seg1Count = 17;

// 'wall 2' (37 vertices)
static const float hole06_turfWall_seg2[][2] = {
    {  12.772f,  13.897f},
    {  12.918f,  14.659f},
    {  12.918f,  15.480f},
    {  13.114f,  16.007f},
    {  13.309f,  16.359f},
    {  13.407f,  16.652f},
    {  13.797f,  16.946f},
    {  14.139f,  17.004f},
    {  14.383f,  17.063f},
    {  14.774f,  17.063f},
    {  15.116f,  17.004f},
    {  15.409f,  17.122f},
    {  15.702f,  16.887f},
    {  15.946f,  16.594f},
    {  16.093f,  16.242f},
    {  16.386f,  15.773f},
    {  16.532f,  15.362f},
    {  16.532f,  14.659f},
    {  16.532f,  14.072f},
    {  16.484f,  13.603f},
    {  16.190f,  13.545f},
    {  16.142f,  14.072f},
    {  16.142f,  14.776f},
    {  16.142f,  15.245f},
    {  16.093f,  15.597f},
    {  15.946f,  16.125f},
    {  15.604f,  16.477f},
    {  15.263f,  16.594f},
    {  14.872f,  16.770f},
    {  14.139f,  16.770f},
    {  13.748f,  16.594f},
    {  13.602f,  16.359f},
    {  13.407f,  15.949f},
    {  13.358f,  15.304f},
    {  13.260f,  14.893f},
    {  13.211f,  14.307f},
    {  13.065f,  13.779f},
};
static const int hole06_turfWall_seg2Count = 37;

// 'wall 3' (24 vertices)
static const float hole06_turfWall_seg3[][2] = {
    {  18.291f,  17.239f},
    {  18.486f,  17.180f},
    {  18.486f,  16.652f},
    {  18.486f,  16.183f},
    {  18.486f,  15.773f},
    {  18.535f,  15.480f},
    {  18.535f,  14.952f},
    {  18.535f,  14.483f},
    {  18.437f,  14.190f},
    {  18.291f,  13.838f},
    {  18.095f,  13.603f},
    {  17.900f,  13.310f},
    {  17.656f,  13.134f},
    {  17.411f,  13.134f},
    {  17.363f,  13.369f},
    {  17.753f,  13.545f},
    {  17.900f,  13.779f},
    {  18.144f,  14.072f},
    {  18.291f,  14.542f},
    {  18.193f,  15.245f},
    {  18.291f,  15.714f},
    {  18.242f,  16.125f},
    {  18.242f,  16.535f},
    {  18.242f,  17.004f},
};
static const int hole06_turfWall_seg3Count = 24;

static void addHole06TurfWallSegments(TurfWall* wall)
{
    wall->addSegment(hole06_turfWall_seg1, hole06_turfWall_seg1Count);
    wall->addSegment(hole06_turfWall_seg2, hole06_turfWall_seg2Count);
    wall->addSegment(hole06_turfWall_seg3, hole06_turfWall_seg3Count);
}
// END HOLE06_TURF_WALL_DATA

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

TurfWall* TurfWall::createHole01Hole09()
{
    constexpr float Y_BASE = 0.007f;
    constexpr float HEIGHT = 0.32f;
    constexpr float R = 0.12f, G = 0.50f, B = 0.14f;

    auto* wall = new TurfWall(Y_BASE, HEIGHT, R, G, B);
    addHole0109TurfWallSegments(wall);
    wall->build();
    return wall;
}

TurfWall* TurfWall::createHole02()
{
    constexpr float Y_BASE = 0.007f;
    constexpr float HEIGHT = 0.32f;
    constexpr float R = 0.12f, G = 0.50f, B = 0.14f;

    auto* wall = new TurfWall(Y_BASE, HEIGHT, R, G, B);
    addHole02TurfWallSegments(wall);
    wall->build();
    return wall;
}

TurfWall* TurfWall::createHole03()
{
    constexpr float Y_BASE = 0.007f;
    constexpr float HEIGHT = 0.32f;
    constexpr float R = 0.12f, G = 0.50f, B = 0.14f;

    auto* wall = new TurfWall(Y_BASE, HEIGHT, R, G, B);
    addHole03TurfWallSegments(wall);
    wall->build();
    return wall;
}

TurfWall* TurfWall::createHole04()
{
    constexpr float Y_BASE = 0.007f;
    constexpr float HEIGHT = 0.32f;
    constexpr float R = 0.12f, G = 0.50f, B = 0.14f;

    auto* wall = new TurfWall(Y_BASE, HEIGHT, R, G, B);
    addHole04TurfWallSegments(wall);
    wall->build();
    return wall;
}

TurfWall* TurfWall::createHole05()
{
    constexpr float Y_BASE = 0.007f;
    constexpr float HEIGHT = 0.32f;
    constexpr float R = 0.12f, G = 0.50f, B = 0.14f;

    auto* wall = new TurfWall(Y_BASE, HEIGHT, R, G, B);
    addHole05TurfWallSegments(wall);
    wall->build();
    return wall;
}

TurfWall* TurfWall::createHole08()
{
    constexpr float Y_BASE = 0.007f;
    constexpr float HEIGHT = 0.32f;
    constexpr float R = 0.12f, G = 0.50f, B = 0.14f;

    auto* wall = new TurfWall(Y_BASE, HEIGHT, R, G, B);
    addHole08TurfWallSegments(wall);
    wall->build();
    return wall;
}

TurfWall* TurfWall::createHole10()
{
    constexpr float Y_BASE = 0.007f;
    constexpr float HEIGHT = 0.32f;
    constexpr float R = 0.12f, G = 0.50f, B = 0.14f;

    auto* wall = new TurfWall(Y_BASE, HEIGHT, R, G, B);
    addHole10TurfWallSegments(wall);
    wall->build();
    return wall;
}

TurfWall* TurfWall::createHole11()
{
    constexpr float Y_BASE = 0.007f;
    constexpr float HEIGHT = 0.32f;
    constexpr float R = 0.12f, G = 0.50f, B = 0.14f;

    auto* wall = new TurfWall(Y_BASE, HEIGHT, R, G, B);
    addHole11TurfWallSegments(wall);
    wall->build();
    return wall;
}

TurfWall* TurfWall::createHole12()
{
    constexpr float Y_BASE = 0.007f;
    constexpr float HEIGHT = 0.32f;
    constexpr float R = 0.12f, G = 0.50f, B = 0.14f;

    auto* wall = new TurfWall(Y_BASE, HEIGHT, R, G, B);
    addHole12TurfWallSegments(wall);
    wall->build();
    return wall;
}

TurfWall* TurfWall::createHole13()
{
    constexpr float Y_BASE = 0.007f;
    constexpr float HEIGHT = 0.32f;
    constexpr float R = 0.12f, G = 0.50f, B = 0.14f;
    auto* wall = new TurfWall(Y_BASE, HEIGHT, R, G, B);
    addHole13TurfWallSegments(wall);
    wall->build();
    return wall;
}

TurfWall* TurfWall::createHole14()
{
    constexpr float Y_BASE = 0.007f;
    constexpr float HEIGHT = 0.32f;
    constexpr float R = 0.12f, G = 0.50f, B = 0.14f;
    auto* wall = new TurfWall(Y_BASE, HEIGHT, R, G, B);
    addHole14TurfWallSegments(wall);
    wall->build();
    return wall;
}

TurfWall* TurfWall::createHole16()
{
    constexpr float Y_BASE = 0.007f;
    constexpr float HEIGHT = 0.32f;
    constexpr float R = 0.12f, G = 0.50f, B = 0.14f;
    auto* wall = new TurfWall(Y_BASE, HEIGHT, R, G, B);
    addHole16TurfWallSegments(wall);
    wall->build();
    return wall;
}

TurfWall* TurfWall::createHole17()
{
    constexpr float Y_BASE = 0.007f;
    constexpr float HEIGHT = 0.32f;
    constexpr float R = 0.12f, G = 0.50f, B = 0.14f;
    auto* wall = new TurfWall(Y_BASE, HEIGHT, R, G, B);
    addHole17TurfWallSegments(wall);
    wall->build();
    return wall;
}

TurfWall* TurfWall::createHole15()
{
    constexpr float Y_BASE = 0.007f;
    constexpr float HEIGHT = 0.32f;
    constexpr float R = 0.12f, G = 0.50f, B = 0.14f;
    auto* wall = new TurfWall(Y_BASE, HEIGHT, R, G, B);
    addHole15TurfWallSegments(wall);
    wall->build();
    return wall;
}

TurfWall* TurfWall::createHole18()
{
    constexpr float Y_BASE = 0.007f;
    constexpr float HEIGHT = 0.32f;
    constexpr float R = 0.12f, G = 0.50f, B = 0.14f;

    auto* wall = new TurfWall(Y_BASE, HEIGHT, R, G, B);
    addHole18TurfWallSegments(wall);
    wall->build();
    return wall;
}

TurfWall* TurfWall::createHole06()
{
    constexpr float Y_BASE = 0.007f;
    constexpr float HEIGHT = 0.32f;
    constexpr float R = 0.12f, G = 0.50f, B = 0.14f;
    auto* wall = new TurfWall(Y_BASE, HEIGHT, R, G, B);
    addHole06TurfWallSegments(wall);
    wall->build();
    return wall;
}

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
