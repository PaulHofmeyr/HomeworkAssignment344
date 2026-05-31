#include "Cylinder.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

Cylinder::Cylinder(float cx, float cy, float cz,
                   float radius, float height,
                   int sectors,
                   float r, float g, float b)
    : Shape(r, g, b),
      cx(cx), cy(cy), cz(cz),
      radius(radius), height(height),
      sectors(sectors < 8 ? 8 : sectors)
{
}

void Cylinder::build()
{
    float yBot = cy - height / 2.0f;
    float yTop = cy + height / 2.0f;

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

        // Filled
        // Side quad
        pushTriangle(filled, x0, yBot, z0, x1, yBot, z1, x1, yTop, z1);
        pushTriangle(filled, x0, yBot, z0, x1, yTop, z1, x0, yTop, z0);

        // Bottom cap triangle
        pushTriangle(filled, cx, yBot, cz, x1, yBot, z1, x0, yBot, z0);

        // Top cap triangle
        pushTriangle(filled, cx, yTop, cz, x0, yTop, z0, x1, yTop, z1);

        // Wireframe

        // Bottom ring edge
        pushLine(wire, x0, yBot, z0, x1, yBot, z1);
        // Top ring edge
        pushLine(wire, x0, yTop, z0, x1, yTop, z1);
        // Vertical edge
        pushLine(wire, x0, yBot, z0, x0, yTop, z0);
        // Bottom cap spoke
        pushLine(wire, cx, yBot, cz, x0, yBot, z0);
        // Top cap spoke
        pushLine(wire, cx, yTop, cz, x0, yTop, z0);
    }

    buildBuffers(filled, wire);
}