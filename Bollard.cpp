#include "Bollard.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

Bollard::Bollard(float cx, float cy, float cz,
                 float r, float g, float b,
                 int sectors)
    : Shape(r, g, b),
      cx(cx), cy(cy), cz(cz),
      sectors(sectors < 8 ? 8 : sectors)
{}

void Bollard::build()
{
    std::vector<float> filled, wire;

    const float R  = BODY_RADIUS;
    const float H  = BODY_HEIGHT;
    const int   S  = sectors;
    const int   DS = (int)DOME_STACKS;

    float yBot = cy;
    float yTop = cy + H;

    // ── Helper lambdas ────────────────────────────────────────────────────────
    auto rx = [&](float radius, int i)
    { return cx + radius * std::cos(i * 2.0f * (float)M_PI / S); };
    auto rz = [&](float radius, int i)
    { return cz + radius * std::sin(i * 2.0f * (float)M_PI / S); };

    // ── Cylinder body ─────────────────────────────────────────────────────────
    for (int i = 0; i < S; i++)
    {
        int  next = (i + 1) % S;
        float x0 = rx(R,i),    z0 = rz(R,i);
        float x1 = rx(R,next), z1 = rz(R,next);

        // Side quad
        pushTriangle(filled, x0,yBot,z0, x1,yBot,z1, x1,yTop,z1);
        pushTriangle(filled, x0,yBot,z0, x1,yTop,z1, x0,yTop,z0);

        // Bottom cap
        pushTriangle(filled, cx,yBot,cz, x1,yBot,z1, x0,yBot,z0);

        // Wireframe
        pushLine(wire, x0,yBot,z0, x1,yBot,z1);
        pushLine(wire, x0,yTop,z0, x1,yTop,z1);
        pushLine(wire, x0,yBot,z0, x0,yTop,z0);
    }

    // ── Hemispherical dome (stacked rings) ────────────────────────────────────
    // Each stack samples a latitude angle from 0 (equator) to pi/2 (apex).
    for (int st = 0; st < DS; st++)
    {
        float phi0 = (float)st       / DS * (float)M_PI * 0.5f;
        float phi1 = (float)(st + 1) / DS * (float)M_PI * 0.5f;

        float r0 = R * std::cos(phi0);   float y0 = yTop + R * std::sin(phi0);
        float r1 = R * std::cos(phi1);   float y1 = yTop + R * std::sin(phi1);

        for (int i = 0; i < S; i++)
        {
            int next = (i + 1) % S;

            float ax = rx(r0,i),    az = rz(r0,i);
            float bx = rx(r0,next), bz = rz(r0,next);
            float cx_ = rx(r1,i),   cz_ = rz(r1,i);
            float dx = rx(r1,next), dz = rz(r1,next);

            if (st < DS - 1) {
                pushTriangle(filled, ax,y0,az, bx,y0,bz, dx,y1,dz);
                pushTriangle(filled, ax,y0,az, dx,y1,dz, cx_,y1,cz_);
            } else {
                // Top cap — converges to apex point
                float apexX = cx, apexY = yTop + R, apexZ = cz;
                pushTriangle(filled, ax,y0,az, bx,y0,bz, apexX,apexY,apexZ);
            }

            // Wireframe rings
            pushLine(wire, ax,y0,az, bx,y0,bz);
            if (st < DS - 1)
                pushLine(wire, ax,y0,az, cx_,y1,cz_);
        }
    }

    buildBuffers(filled, wire);
}
