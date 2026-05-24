#ifndef LIGHTING_H
#define LIGHTING_H

#include <GL/glew.h>
#include <string>
#include <cmath>
#include <vector>
#include "AppState.h"
#include "Transformations.h"

// ─────────────────────────────────────────────────────────────────────────────
// HOW TO ADD A NEW SPOTLIGHT
// ─────────────────────────────────────────────────────────────────────────────
// 1. Add an entry to the `spotDefs` vector in Lighting's constructor below.
//    Fill in position, direction, inner/outer cut-off (radians), and whether
//    it's always on or only on at dusk/night.
//
// 2. That's it. Shadow maps, uniforms, and depth passes are all automatic.
//
// SpotDef fields:
//   pos        – world-space position  (x, y, z)
//   dir        – normalised direction the cone points  (x, y, z)
//   innerRad   – inner cut-off angle in radians  (fully lit inside)
//   outerRad   – outer cut-off angle in radians  (fully dark outside)
//   diffuse    – RGB colour/intensity of the light
//   specular   – RGB specular contribution
//   constant/linear/quadratic – attenuation coefficients
//   onAtDusk   – if true, light activates at DUSK and NIGHT;
//                if false, light is always enabled
//   castShadow – if true, a shadow map is generated for this light
// ─────────────────────────────────────────────────────────────────────────────

struct Vec3 { float x, y, z; };

struct SpotDef
{
    Vec3  pos;
    Vec3  dir;
    float innerRad   = 0.261f;   // ~15°
    float outerRad   = 0.436f;   // ~25°
    Vec3  ambient    = {0.01f, 0.01f, 0.01f};
    Vec3  diffuse    = {0.95f, 0.95f, 0.90f};
    Vec3  specular   = {0.50f, 0.50f, 0.50f};
    float constant   = 1.0f;
    float linear     = 0.027f;
    float quadratic  = 0.0028f;
    bool  onAtDusk   = true;    // activate at dusk/night
    bool  castShadow = true;
};

// ── Uniform helpers ───────────────────────────────────────────────────────────
static inline void setUniformBool(GLuint p, const std::string& n, bool v)
{ glUniform1i(glGetUniformLocation(p, n.c_str()), v ? 1 : 0); }
static inline void setUniform1f(GLuint p, const std::string& n, float v)
{ glUniform1f(glGetUniformLocation(p, n.c_str()), v); }
static inline void setUniform1i(GLuint p, const std::string& n, int v)
{ glUniform1i(glGetUniformLocation(p, n.c_str()), v); }

// MAX_SPOT_SHADOWS: must match the #define in fragmentShader.glsl
static constexpr int MAX_SPOT_SHADOWS = 16;
static constexpr int MAX_SPOTLIGHTS    = 32;

class Lighting
{
public:
    // ── Point light positions ─────────────────────────────────────────────────
    static constexpr int NUM_BOLLARDS = 2;
    Vec3 bollardPos[NUM_BOLLARDS] = {
        { -3.0f, 0.4f,  0.0f },
        {  3.0f, 0.4f,  0.0f },
    };
    Vec3 gazePos = { 0.0f, 2.8f, 0.0f };

    // ── Spotlight definitions ─────────────────────────────────────────────────
    // Add new spotlights here — everything else is automatic.
    std::vector<SpotDef> spotDefs;

    // ── Light-space matrices (built each frame, read by main.cpp) ─────────────
    Matrix<4,4> spotLSMs[MAX_SPOT_SHADOWS];
    int         spotShadowLayer[MAX_SPOTLIGHTS];
    int         numSpotShadows = 0;

    // ─────────────────────────────────────────────────────────────────────────
    Lighting()
    {
        // ── Floodlight clusters (4 poles × 3 spots) ───────────────────────────
        Vec3 floodPos[4] = {
            { -8.0f, 6.0f,  8.0f },
            {  8.0f, 6.0f,  8.0f },
            { -8.0f, 6.0f, -8.0f },
            {  8.0f, 6.0f, -8.0f },
        };
        float spreads[3] = { 0.0f, 0.25f, -0.25f };

        for (int p = 0; p < 4; p++) {
            for (int s = 0; s < 3; s++) {
                SpotDef d;
                d.pos       = floodPos[p];
                d.dir       = { std::sin(spreads[s]), -1.0f, std::cos(spreads[s]) * 0.05f };
                d.innerRad  = 0.261f;   // 15°
                d.outerRad  = 0.436f;   // 25°
                d.ambient   = {0.01f, 0.01f, 0.01f};
                d.diffuse   = {0.95f, 0.95f, 0.90f};
                d.specular  = {0.50f, 0.50f, 0.50f};
                d.constant  = 1.0f;
                d.linear    = 0.027f;
                d.quadratic = 0.0028f;
                d.onAtDusk  = true;
                d.castShadow = true;
                spotDefs.push_back(d);
            }
        }

        // ── ADD YOUR OWN SPOTLIGHTS BELOW THIS LINE ───────────────────────────
        // Example (copy, uncomment, and edit):
        //
        // SpotDef mySpot;
        // mySpot.pos        = { 0.0f, 3.0f, 0.0f };   // position
        // mySpot.dir        = { 0.0f, -1.0f, 0.0f };  // pointing straight down
        // mySpot.innerRad   = 0.2f;                    // ~11° inner cone
        // mySpot.outerRad   = 0.35f;                   // ~20° outer cone
        // mySpot.diffuse    = { 1.0f, 0.8f, 0.4f };   // warm yellow
        // mySpot.specular   = { 0.5f, 0.4f, 0.2f };
        // mySpot.onAtDusk   = true;
        // mySpot.castShadow = true;
        // spotDefs.push_back(mySpot);
        //
        // ── END OF USER SPOTLIGHTS ────────────────────────────────────────────

        // NOTE: the drone spotlight is added dynamically each frame in upload()
        // and is always the last entry — do not add it manually here.
    }

    // ── Called once per frame from main.cpp ───────────────────────────────────
    // Also rebuilds spotLSMs[] so main.cpp can run the depth passes.
    void upload(GLuint prog, const AppState& app,
                float droneX, float droneY, float droneZ,
                float droneFwdX, float droneFwdY, float droneFwdZ)
    {
        bool isNight    = (app.timeState == TimeState::NIGHT);
        bool isDusk     = (app.timeState == TimeState::DUSK);
        bool fixturesOn = (isDusk || isNight);

        // ── Sun ───────────────────────────────────────────────────────────────
        glUniform3f(glGetUniformLocation(prog, "sun.direction"), 0.5f, 1.0f, 0.3f);
        if (isNight) {
            glUniform3f(glGetUniformLocation(prog, "sun.ambient"),  0.05f, 0.05f, 0.09f);
            glUniform3f(glGetUniformLocation(prog, "sun.diffuse"),  0.0f,  0.0f,  0.0f);
            glUniform3f(glGetUniformLocation(prog, "sun.specular"), 0.0f,  0.0f,  0.0f);
            setUniformBool(prog, "sun.enabled", false);
        } else if (isDusk) {
            glUniform3f(glGetUniformLocation(prog, "sun.ambient"),  0.12f, 0.09f, 0.05f);
            glUniform3f(glGetUniformLocation(prog, "sun.diffuse"),  0.70f, 0.45f, 0.20f);
            glUniform3f(glGetUniformLocation(prog, "sun.specular"), 0.50f, 0.35f, 0.15f);
            setUniformBool(prog, "sun.enabled", true);
        } else {
            glUniform3f(glGetUniformLocation(prog, "sun.ambient"),  0.20f, 0.20f, 0.20f);
            glUniform3f(glGetUniformLocation(prog, "sun.diffuse"),  0.90f, 0.90f, 0.85f);
            glUniform3f(glGetUniformLocation(prog, "sun.specular"), 0.70f, 0.70f, 0.70f);
            setUniformBool(prog, "sun.enabled", true);
        }

        // ── Point lights ──────────────────────────────────────────────────────
        int ptIdx = 0;
        for (int i = 0; i < NUM_BOLLARDS; i++, ptIdx++) {
            std::string b = "pointLights[" + std::to_string(ptIdx) + "]";
            glUniform3f(glGetUniformLocation(prog, (b+".position").c_str()),
                        bollardPos[i].x, bollardPos[i].y, bollardPos[i].z);
            glUniform3f(glGetUniformLocation(prog, (b+".ambient").c_str()),  0.02f, 0.015f, 0.01f);
            glUniform3f(glGetUniformLocation(prog, (b+".diffuse").c_str()),  0.60f, 0.50f,  0.35f);
            glUniform3f(glGetUniformLocation(prog, (b+".specular").c_str()), 0.20f, 0.15f,  0.10f);
            glUniform1f(glGetUniformLocation(prog, (b+".constant").c_str()),  1.0f);
            glUniform1f(glGetUniformLocation(prog, (b+".linear").c_str()),    0.35f);
            glUniform1f(glGetUniformLocation(prog, (b+".quadratic").c_str()), 0.44f);
            setUniformBool(prog, b+".enabled", fixturesOn);
        }
        {
            std::string b = "pointLights[" + std::to_string(ptIdx) + "]";
            glUniform3f(glGetUniformLocation(prog, (b+".position").c_str()),
                        gazePos.x, gazePos.y, gazePos.z);
            glUniform3f(glGetUniformLocation(prog, (b+".ambient").c_str()),  0.02f, 0.02f, 0.02f);
            glUniform3f(glGetUniformLocation(prog, (b+".diffuse").c_str()),  0.80f, 0.78f, 0.70f);
            glUniform3f(glGetUniformLocation(prog, (b+".specular").c_str()), 0.30f, 0.30f, 0.25f);
            glUniform1f(glGetUniformLocation(prog, (b+".constant").c_str()),  1.0f);
            glUniform1f(glGetUniformLocation(prog, (b+".linear").c_str()),    0.09f);
            glUniform1f(glGetUniformLocation(prog, (b+".quadratic").c_str()), 0.032f);
            setUniformBool(prog, b+".enabled", fixturesOn);
            ptIdx++;
        }
        setUniform1i(prog, "numPointLights", ptIdx);

        // ── Spotlights (all entries in spotDefs, then drone last) ─────────────
        numSpotShadows = 0;
        int spIdx = 0;
        for (int i = 0; i < MAX_SPOTLIGHTS; i++) spotShadowLayer[i] = -1;

        for (const SpotDef& d : spotDefs) {
            _uploadSpot(prog, spIdx, d, fixturesOn ? d.onAtDusk : !d.onAtDusk);
            if (d.castShadow && numSpotShadows < MAX_SPOT_SHADOWS) {
                spotShadowLayer[spIdx] = numSpotShadows;
                spotLSMs[numSpotShadows++] = _buildSpotLSM(d.pos, d.dir, d.outerRad);
            }
            spIdx++;
        }

        // ── Drone spotlight (always last) ─────────────────────────────────────
        {
            SpotDef drone;
            drone.pos       = {droneX, droneY, droneZ};
            drone.dir       = {droneFwdX, droneFwdY, droneFwdZ};
            drone.innerRad  = 0.175f;    // 10°
            drone.outerRad  = 0.349f;    // 20°
            drone.ambient   = {0.0f,  0.0f,  0.0f};
            drone.diffuse   = {1.0f,  1.0f,  0.95f};
            drone.specular  = {0.80f, 0.80f, 0.80f};
            drone.constant  = 1.0f;
            drone.linear    = 0.045f;
            drone.quadratic = 0.0075f;
            drone.onAtDusk  = false;
            drone.castShadow = true;

            _uploadSpot(prog, spIdx, drone, app.droneLights);

            if (numSpotShadows < MAX_SPOT_SHADOWS) {
                spotShadowLayer[spIdx] = numSpotShadows;
                spotLSMs[numSpotShadows++] = _buildSpotLSM(drone.pos, drone.dir, drone.outerRad);
            }

            spIdx++;
        }

        setUniform1i(prog, "numSpotLights",  spIdx);
        for (int i = 0; i < spIdx && i < MAX_SPOTLIGHTS; i++) {
            std::string uname = "spotShadowLayer[" + std::to_string(i) + "]";
            setUniform1i(prog, uname.c_str(), spotShadowLayer[i]);
        }
        // Tell the fragment shader how many shadow maps are valid
        setUniform1i(prog, "numSpotShadows", numSpotShadows);

        // ── Material defaults ─────────────────────────────────────────────────
        setUniformBool(prog, "useMaterialOverride", false);
        glUniform3f(glGetUniformLocation(prog, "material.ambient"),  0.2f, 0.2f, 0.2f);
        glUniform3f(glGetUniformLocation(prog, "material.specular"), 0.3f, 0.3f, 0.3f);
        setUniform1f(prog, "material.shininess", 16.0f);
    }

private:
    // Upload one spotlight's uniforms at index spIdx
    void _uploadSpot(GLuint prog, int spIdx, const SpotDef& d, bool enabled)
    {
        std::string b = "spotLights[" + std::to_string(spIdx) + "]";

        // Normalise direction before uploading
        float len = std::sqrt(d.dir.x*d.dir.x + d.dir.y*d.dir.y + d.dir.z*d.dir.z);
        if (len < 1e-5f) len = 1.0f;

        glUniform3f(glGetUniformLocation(prog, (b+".position").c_str()),
                    d.pos.x, d.pos.y, d.pos.z);
        glUniform3f(glGetUniformLocation(prog, (b+".direction").c_str()),
                    d.dir.x/len, d.dir.y/len, d.dir.z/len);
        glUniform3f(glGetUniformLocation(prog, (b+".ambient").c_str()),
                    d.ambient.x, d.ambient.y, d.ambient.z);
        glUniform3f(glGetUniformLocation(prog, (b+".diffuse").c_str()),
                    d.diffuse.x, d.diffuse.y, d.diffuse.z);
        glUniform3f(glGetUniformLocation(prog, (b+".specular").c_str()),
                    d.specular.x, d.specular.y, d.specular.z);
        glUniform1f(glGetUniformLocation(prog, (b+".cutOffInner").c_str()), std::cos(d.innerRad));
        glUniform1f(glGetUniformLocation(prog, (b+".cutOffOuter").c_str()), std::cos(d.outerRad));
        glUniform1f(glGetUniformLocation(prog, (b+".constant").c_str()),   d.constant);
        glUniform1f(glGetUniformLocation(prog, (b+".linear").c_str()),     d.linear);
        glUniform1f(glGetUniformLocation(prog, (b+".quadratic").c_str()),  d.quadratic);
        setUniformBool(prog, b+".enabled", enabled);
    }

    // Build a perspective light-space matrix for a spotlight
    Matrix<4,4> _buildSpotLSM(Vec3 pos, Vec3 dir, float outerRad)
    {
        float len = std::sqrt(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
        if (len < 1e-5f) len = 1.0f;
        float nx = dir.x/len, ny = dir.y/len, nz = dir.z/len;

        // Up vector — avoid collinearity with forward
        float upX = 0.f, upY = 1.f, upZ = 0.f;
        if (std::fabs(ny) > 0.99f) { upY = 0.f; upZ = 1.f; }

        Matrix<4,4> lView = makeLookAt(
            pos.x, pos.y, pos.z,
            pos.x + nx, pos.y + ny, pos.z + nz,
            upX, upY, upZ);

        // FOV = 2 × outer cut-off + small margin so the full cone is covered
        Matrix<4,4> lProj = makePerspective(outerRad * 2.0f + 0.1f, 1.0f, 0.1f, 50.0f);
        return lProj * lView;
    }
};

#endif // LIGHTING_H
