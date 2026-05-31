#include "TriangularPrism.h"

TriangularPrism::TriangularPrism(float cx, float cy, float cz,
                                 float hw, float hh, float tz,
                                 float r, float g, float b)
    : Shape(r, g, b),
      cx(cx), cy(cy), cz(cz),
      hw(hw), hh(hh), tz(tz)
{
}

void TriangularPrism::build()
{
    // Bottom triangle
    float ax = cx, ay = cy - hh, az = cz + tz;         // front tip
    float bx = cx - hw, by = cy - hh, bz = cz - tz;    // back left
    float cx_ = cx + hw, cy_ = cy - hh, cz_ = cz - tz; // back right
    // Top triangle
    float dx = cx, dy = cy + hh, dz = cz + tz;      // front tip
    float ex = cx - hw, ey = cy + hh, ez = cz - tz; // back left
    float fx = cx + hw, fy = cy + hh, fz = cz - tz; // back right

    std::vector<float> filled, wire;

    // Bottom triangle
    pushTriangle(filled, ax, ay, az, cx_, cy_, cz_, bx, by, bz);
    // Top triangle
    pushTriangle(filled, dx, dy, dz, ex, ey, ez, fx, fy, fz);
    // Front face
    pushTriangle(filled, ax, ay, az, dx, dy, dz, ex, ey, ez);
    pushTriangle(filled, ax, ay, az, ex, ey, ez, bx, by, bz);
    // Right face
    pushTriangle(filled, cx_, cy_, cz_, fx, fy, fz, dx, dy, dz);
    pushTriangle(filled, cx_, cy_, cz_, dx, dy, dz, ax, ay, az);
    // Back face
    pushTriangle(filled, bx, by, bz, ex, ey, ez, fx, fy, fz);
    pushTriangle(filled, bx, by, bz, fx, fy, fz, cx_, cy_, cz_);

    // Bottom triangle edges
    pushLine(wire, ax, ay, az, bx, by, bz);
    pushLine(wire, bx, by, bz, cx_, cy_, cz_);
    pushLine(wire, cx_, cy_, cz_, ax, ay, az);
    // Top triangle edges
    pushLine(wire, dx, dy, dz, ex, ey, ez);
    pushLine(wire, ex, ey, ez, fx, fy, fz);
    pushLine(wire, fx, fy, fz, dx, dy, dz);
    // Vertical edges
    pushLine(wire, ax, ay, az, dx, dy, dz);
    pushLine(wire, bx, by, bz, ex, ey, ez);
    pushLine(wire, cx_, cy_, cz_, fx, fy, fz);

    buildBuffers(filled, wire);
}