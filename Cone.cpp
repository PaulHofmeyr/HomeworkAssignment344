#include "Cone.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

Cone::Cone(float cx, float cy, float cz,
           float radius, float height,
           int sectors,
           float r, float g, float b)
    : Shape(r, g, b),
      cx(cx), cy(cy), cz(cz),
      radius(radius), height(height),
      sectors(sectors < 8 ? 8 : sectors)
{
}

void Cone::build()
{
    float yBase = cy;
    float yApex = cy + height;

    std::vector<float> filled, wire;

    auto rx = [&](int i)
    { return cx + radius * std::cos(i * 2.0f * (float)M_PI / sectors); };
    auto rz = [&](int i)
    { return cz + radius * std::sin(i * 2.0f * (float)M_PI / sectors); };

    for (int i = 0; i < sectors; i++)
    {
        int next = (i + 1) % sectors;

        float x0 = rx(i), z0 = rz(i);
        float x1 = rx(next), z1 = rz(next);

        // Side triangle
        pushTriangle(filled, x0, yBase, z0, x1, yBase, z1, cx, yApex, cz);

        // Base cap
        pushTriangle(filled, cx, yBase, cz, x1, yBase, z1, x0, yBase, z0);

        // Wireframe

        // Base ring edge
        pushLine(wire, x0, yBase, z0, x1, yBase, z1);
        // Slant edge to apex
        pushLine(wire, x0, yBase, z0, cx, yApex, cz);
        // Base cap spoke
        pushLine(wire, cx, yBase, cz, x0, yBase, z0);
    }

    buildBuffers(filled, wire);
}