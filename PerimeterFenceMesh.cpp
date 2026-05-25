#include "PerimeterFenceMesh.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

namespace {

constexpr float FENCE_R     = 0.05f, FENCE_G = 0.05f, FENCE_B = 0.05f;
constexpr float RAIL_BOT    = 0.18f;
constexpr float RAIL_TOP    = 0.82f;
constexpr float POST_RADIUS = 0.04f;
constexpr float POST_HEIGHT = 1.0f;
constexpr int   PICKETS_PER_BAY = 18;
constexpr int   POST_SIDES    = 6;

int postsAlongLength(float len, float spacing)
{
    if(len < 1e-4f)
        return 1;
    return std::max(2, (int)std::ceil(len / spacing) + 1);
}

} // namespace

PerimeterFenceMesh::PerimeterFenceMesh()
    : Shape(FENCE_R, FENCE_G, FENCE_B)
{}

void PerimeterFenceMesh::appendBox(std::vector<float>& buf,
                                   float cx, float cy, float cz,
                                   float hx, float hy, float hz,
                                   float rotY) const
{
    const float c = std::cos(rotY);
    const float s = std::sin(rotY);

    auto rot = [&](float lx, float ly, float lz,
                   float& wx, float& wy, float& wz) {
        wx = cx + c * lx + s * lz;
        wy = cy + ly;
        wz = cz - s * lx + c * lz;
    };

    float x0, y0, z0, x1, y1, z1, x2, y2, z2, x3, y3, z3;
    float x4, y4, z4, x5, y5, z5, x6, y6, z6, x7, y7, z7;

    rot(-hx, -hy, -hz, x0, y0, z0);
    rot( hx, -hy, -hz, x1, y1, z1);
    rot( hx, -hy,  hz, x2, y2, z2);
    rot(-hx, -hy,  hz, x3, y3, z3);
    rot(-hx,  hy, -hz, x4, y4, z4);
    rot( hx,  hy, -hz, x5, y5, z5);
    rot( hx,  hy,  hz, x6, y6, z6);
    rot(-hx,  hy,  hz, x7, y7, z7);

    auto tri = [&](float ax, float ay, float az,
                   float bx, float by, float bz,
                   float cpx, float cpy, float cpz) {
        pushTriangle(buf, ax, ay, az, bx, by, bz, cpx, cpy, cpz);
    };

    tri(x0, y0, z0, x2, y2, z2, x1, y1, z1);
    tri(x0, y0, z0, x3, y3, z3, x2, y2, z2);
    tri(x4, y4, z4, x5, y5, z5, x6, y6, z6);
    tri(x4, y4, z4, x6, y6, z6, x7, y7, z7);
    tri(x0, y0, z0, x1, y1, z1, x5, y5, z5);
    tri(x0, y0, z0, x5, y5, z5, x4, y4, z4);
    tri(x3, y3, z3, x2, y2, z2, x6, y6, z6);
    tri(x3, y3, z3, x6, y6, z6, x7, y7, z7);
    tri(x0, y0, z0, x4, y4, z4, x7, y7, z7);
    tri(x0, y0, z0, x7, y7, z7, x3, y3, z3);
    tri(x1, y1, z1, x2, y2, z2, x6, y6, z6);
    tri(x1, y1, z1, x6, y6, z6, x5, y5, z5);
}

void PerimeterFenceMesh::appendPost(std::vector<float>& buf, float px, float pz) const
{
    const float yBot = 0.f;
    const float yTop = POST_HEIGHT;

    for(int i = 0; i < POST_SIDES; ++i)
    {
        float a0 = (float)i / POST_SIDES * 2.f * M_PI;
        float a1 = (float)(i + 1) / POST_SIDES * 2.f * M_PI;
        float x0 = px + POST_RADIUS * std::cos(a0);
        float z0 = pz + POST_RADIUS * std::sin(a0);
        float x1 = px + POST_RADIUS * std::cos(a1);
        float z1 = pz + POST_RADIUS * std::sin(a1);
        pushTriangle(buf, px, yBot, pz, x1, yBot, z1, x0, yBot, z0);
        pushTriangle(buf, px, yTop, pz, x0, yTop, z0, x1, yTop, z1);
        pushTriangle(buf, x0, yBot, z0, x1, yBot, z1, x1, yTop, z1);
        pushTriangle(buf, x0, yBot, z0, x1, yTop, z1, x0, yTop, z0);
    }
}

void PerimeterFenceMesh::buildMesh(float xMin, float xMax, float zMin, float zMax, float spacing)
{
    std::vector<float> buf;
    buf.reserve(500000);

    const struct Edge { float x0, z0, x1, z1; } edges[] = {
        {xMin, zMin, xMax, zMin},
        {xMax, zMin, xMax, zMax},
        {xMax, zMax, xMin, zMax},
        {xMin, zMax, xMin, zMin},
    };

    constexpr float railHalfY = 0.02f;
    constexpr float railHalfD = 0.02f;
    constexpr float picketHalfW = 0.006f;
    constexpr float picketHalfH = (RAIL_TOP - RAIL_BOT) * 0.5f;

    for(const Edge& e : edges)
    {
        float dx = e.x1 - e.x0;
        float dz = e.z1 - e.z0;
        float len = std::sqrt(dx * dx + dz * dz);
        if(len < 1e-4f)
            continue;

        int posts = postsAlongLength(len, spacing);

        for(int i = 0; i < posts; ++i)
        {
            float t = (posts > 1) ? (float)i / (float)(posts - 1) : 0.f;
            appendPost(buf, e.x0 + t * dx, e.z0 + t * dz);
        }

        for(int i = 0; i < posts - 1; ++i)
        {
            float t0 = (float)i / (float)(posts - 1);
            float t1 = (float)(i + 1) / (float)(posts - 1);
            float sx = e.x0 + t0 * dx;
            float sz = e.z0 + t0 * dz;
            float ex = e.x0 + t1 * dx;
            float ez = e.z0 + t1 * dz;

            float segDx = ex - sx;
            float segDz = ez - sz;
            float segLen = std::sqrt(segDx * segDx + segDz * segDz);
            if(segLen < 1e-4f)
                continue;

            float angle = std::atan2(-segDz, segDx);
            float midX  = (sx + ex) * 0.5f;
            float midZ  = (sz + ez) * 0.5f;
            float halfLen = segLen * 0.5f;

            appendBox(buf, midX, RAIL_BOT, midZ, halfLen, railHalfY, railHalfD, angle);
            appendBox(buf, midX, RAIL_TOP, midZ, halfLen, railHalfY, railHalfD, angle);

            for(int k = 1; k <= PICKETS_PER_BAY; ++k)
            {
                float t = (float)k / (float)(PICKETS_PER_BAY + 1);
                float px = sx + t * segDx;
                float pz = sz + t * segDz;
                appendBox(buf, px, RAIL_BOT + picketHalfH, pz,
                          picketHalfW, picketHalfH, picketHalfW, angle);
            }
        }
    }

    buildBuffers(buf, {});
}

PerimeterFenceMesh* PerimeterFenceMesh::create(
    float xMin, float xMax, float zMin, float zMax, float postSpacing)
{
    auto* mesh = new PerimeterFenceMesh();
    mesh->buildMesh(xMin, xMax, zMin, zMax, postSpacing);
    return mesh;
}
