#include "Cuboid.h"

Cuboid::Cuboid(float cx, float cy, float cz,
               float hw, float hh, float hd,
               float r, float g, float b)
    : Shape(r, g, b),
      cx(cx), cy(cy), cz(cz),
      hw(hw), hh(hh), hd(hd)
{
}

void Cuboid::build()
{
    float x0 = cx - hw, x1 = cx + hw;
    float y0 = cy - hh, y1 = cy + hh;
    float z0 = cz - hd, z1 = cz + hd;

    std::vector<float> filled, wire;

    // Front
    pushTriangle(filled, x0, y0, z1, x1, y0, z1, x1, y1, z1);
    pushTriangle(filled, x0, y0, z1, x1, y1, z1, x0, y1, z1);
    // Back
    pushTriangle(filled, x1, y0, z0, x0, y0, z0, x0, y1, z0);
    pushTriangle(filled, x1, y0, z0, x0, y1, z0, x1, y1, z0);
    // Left
    pushTriangle(filled, x0, y0, z0, x0, y0, z1, x0, y1, z1);
    pushTriangle(filled, x0, y0, z0, x0, y1, z1, x0, y1, z0);
    // Right
    pushTriangle(filled, x1, y0, z1, x1, y0, z0, x1, y1, z0);
    pushTriangle(filled, x1, y0, z1, x1, y1, z0, x1, y1, z1);
    // Top
    pushTriangle(filled, x0, y1, z1, x1, y1, z1, x1, y1, z0);
    pushTriangle(filled, x0, y1, z1, x1, y1, z0, x0, y1, z0);
    // Bottom
    pushTriangle(filled, x0, y0, z0, x1, y0, z0, x1, y0, z1);
    pushTriangle(filled, x0, y0, z0, x1, y0, z1, x0, y0, z1);

    // Wireframe:
    // Bottom face
    pushLine(wire, x0, y0, z0, x1, y0, z0);
    pushLine(wire, x1, y0, z0, x1, y0, z1);
    pushLine(wire, x1, y0, z1, x0, y0, z1);
    pushLine(wire, x0, y0, z1, x0, y0, z0);
    // Top face
    pushLine(wire, x0, y1, z0, x1, y1, z0);
    pushLine(wire, x1, y1, z0, x1, y1, z1);
    pushLine(wire, x1, y1, z1, x0, y1, z1);
    pushLine(wire, x0, y1, z1, x0, y1, z0);
    // Verticals
    pushLine(wire, x0, y0, z0, x0, y1, z0);
    pushLine(wire, x1, y0, z0, x1, y1, z0);
    pushLine(wire, x1, y0, z1, x1, y1, z1);
    pushLine(wire, x0, y0, z1, x0, y1, z1);

    buildBuffers(filled, wire);
}