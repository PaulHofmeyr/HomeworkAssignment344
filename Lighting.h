#ifndef LIGHTING_H
#define LIGHTING_H

#include <GL/glew.h>
#include <string>
#include <cmath>
#include "AppState.h"

// Lighting – uploads all light uniforms to the scene shader each frame.
//
// Light sources:
//   • sun          : directional (day) – disabled at night
//   • moonAmbient  : baked into sun ambient when TimeState == NIGHT
//   • pointLights  : bollards (2), under-gazebo (1) – on at DUSK/NIGHT
//   • spotLights   : floodlight clusters (4 poles × 3 spots each = 12)
//                    + drone spotlight (index 12) – when droneLights == true
//
// Floodlight layout

struct Vec3 { float x, y, z; };

//  Helper: set a single uniform string 
static inline void setUniformBool(GLuint prog, const std::string& name, bool v)
{
    glUniform1i(glGetUniformLocation(prog, name.c_str()), v ? 1 : 0);
}
static inline void setUniform3f(GLuint prog, const std::string& name, float x, float y, float z)
{
    glUniform3f(glGetUniformLocation(prog, name.c_str()), x, y, z);
}
static inline void setUniform1f(GLuint prog, const std::string& name, float v)
{
    glUniform1f(glGetUniformLocation(prog, name.c_str()), v);
}
static inline void setUniform1i(GLuint prog, const std::string& name, int v)
{
    glUniform1i(glGetUniformLocation(prog, name.c_str()), v);
}

class Lighting
{
public:
    // Floodlight poles (world positions – tops of 6 m poles)
    static constexpr int NUM_FLOOD_POLES = 4;
    Vec3 floodPos[NUM_FLOOD_POLES] = {
        { -8.0f, 6.0f,  8.0f },
        {  8.0f, 6.0f,  8.0f },
        { -8.0f, 6.0f, -8.0f },
        {  8.0f, 6.0f, -8.0f },
    };

    // Bollard positions 
    static constexpr int NUM_BOLLARDS = 2;
    Vec3 bollardPos[NUM_BOLLARDS] = {
        { -3.0f, 0.4f,  0.0f },
        {  3.0f, 0.4f,  0.0f },
    };

    // Under-gazebo point light
    Vec3 gazePos = { 0.0f, 2.8f, 0.0f };

    // Upload all uniforms 
    void upload(GLuint prog, const AppState& app,
                float droneX, float droneY, float droneZ,
                float droneFwdX, float droneFwdY, float droneFwdZ)
    {
        bool isNight = (app.timeState == TimeState::NIGHT);
        bool isDusk  = (app.timeState == TimeState::DUSK);
        bool fixturesOn = (isDusk || isNight);

        // Sun (directional)
        glUniform3f(glGetUniformLocation(prog, "sun.direction"),
                    0.5f, 1.0f, 0.3f);   // toward the light

        if (isNight)
        {
            // Disabled; only moonlight ambient
            glUniform3f(glGetUniformLocation(prog, "sun.ambient"),  0.05f, 0.05f, 0.09f);
            glUniform3f(glGetUniformLocation(prog, "sun.diffuse"),  0.0f,  0.0f,  0.0f);
            glUniform3f(glGetUniformLocation(prog, "sun.specular"), 0.0f,  0.0f,  0.0f);
            setUniformBool(prog, "sun.enabled", false);
        }
        else if (isDusk)
        {
            // Warm amber, reduced intensity
            glUniform3f(glGetUniformLocation(prog, "sun.ambient"),  0.12f, 0.09f, 0.05f);
            glUniform3f(glGetUniformLocation(prog, "sun.diffuse"),  0.70f, 0.45f, 0.20f);
            glUniform3f(glGetUniformLocation(prog, "sun.specular"), 0.50f, 0.35f, 0.15f);
            setUniformBool(prog, "sun.enabled", true);
        }
        else
        {
            // Midday - bright neutral white
            glUniform3f(glGetUniformLocation(prog, "sun.ambient"),  0.20f, 0.20f, 0.20f);
            glUniform3f(glGetUniformLocation(prog, "sun.diffuse"),  0.90f, 0.90f, 0.85f);
            glUniform3f(glGetUniformLocation(prog, "sun.specular"), 0.70f, 0.70f, 0.70f);
            setUniformBool(prog, "sun.enabled", true);
        }

        // Point lights: bollards + gazebo 
        int ptIdx = 0;

        // Bollards
        for (int i = 0; i < NUM_BOLLARDS; i++, ptIdx++)
        {
            std::string base = "pointLights[" + std::to_string(ptIdx) + "]";
            glUniform3f(glGetUniformLocation(prog, (base+".position").c_str()),
                        bollardPos[i].x, bollardPos[i].y, bollardPos[i].z);
            glUniform3f(glGetUniformLocation(prog, (base+".ambient").c_str()),
                        0.02f, 0.015f, 0.01f);
            glUniform3f(glGetUniformLocation(prog, (base+".diffuse").c_str()),
                        0.60f, 0.50f, 0.35f);   // warm glow
            glUniform3f(glGetUniformLocation(prog, (base+".specular").c_str()),
                        0.20f, 0.15f, 0.10f);
            glUniform1f(glGetUniformLocation(prog, (base+".constant").c_str()),  1.0f);
            glUniform1f(glGetUniformLocation(prog, (base+".linear").c_str()),    0.35f);
            glUniform1f(glGetUniformLocation(prog, (base+".quadratic").c_str()), 0.44f);
            setUniformBool(prog, base+".enabled", fixturesOn);
        }

        // Gazebo light
        {
            std::string base = "pointLights[" + std::to_string(ptIdx) + "]";
            glUniform3f(glGetUniformLocation(prog, (base+".position").c_str()),
                        gazePos.x, gazePos.y, gazePos.z);
            glUniform3f(glGetUniformLocation(prog, (base+".ambient").c_str()),
                        0.02f, 0.02f, 0.02f);
            glUniform3f(glGetUniformLocation(prog, (base+".diffuse").c_str()),
                        0.80f, 0.78f, 0.70f);
            glUniform3f(glGetUniformLocation(prog, (base+".specular").c_str()),
                        0.30f, 0.30f, 0.25f);
            glUniform1f(glGetUniformLocation(prog, (base+".constant").c_str()),  1.0f);
            glUniform1f(glGetUniformLocation(prog, (base+".linear").c_str()),    0.09f);
            glUniform1f(glGetUniformLocation(prog, (base+".quadratic").c_str()), 0.032f);
            setUniformBool(prog, base+".enabled", fixturesOn);
            ptIdx++;
        }

        setUniform1i(prog, "numPointLights", ptIdx);

        // Spotlights: floodlight clusters 
        int spIdx = 0;
        static const float spreadAngles[] = { 0.0f, 0.25f, -0.25f };   // slight yaw spread per spot

        for (int p = 0; p < NUM_FLOOD_POLES; p++)
        {
            Vec3 fp = floodPos[p];
            for (int s = 0; s < 3; s++, spIdx++)
            {
                std::string base = "spotLights[" + std::to_string(spIdx) + "]";

                // Slight horizontal spread per sub-spot
                float spread = spreadAngles[s];
                float dirX = std::sin(spread);
                float dirY = -1.0f;   // pointing down
                float dirZ = std::cos(spread) * 0.05f;

                glUniform3f(glGetUniformLocation(prog, (base+".position").c_str()),
                            fp.x, fp.y, fp.z);
                glUniform3f(glGetUniformLocation(prog, (base+".direction").c_str()),
                            dirX, dirY, dirZ);
                glUniform3f(glGetUniformLocation(prog, (base+".ambient").c_str()),
                            0.01f, 0.01f, 0.01f);
                glUniform3f(glGetUniformLocation(prog, (base+".diffuse").c_str()),
                            0.95f, 0.95f, 0.90f);   // cool white LED
                glUniform3f(glGetUniformLocation(prog, (base+".specular").c_str()),
                            0.50f, 0.50f, 0.50f);
                // Inner cone ~15°, outer ~25° → realistic falloff edge
                glUniform1f(glGetUniformLocation(prog, (base+".cutOffInner").c_str()),
                            std::cos(0.261f));   // 15°
                glUniform1f(glGetUniformLocation(prog, (base+".cutOffOuter").c_str()),
                            std::cos(0.436f));   // 25°
                glUniform1f(glGetUniformLocation(prog, (base+".constant").c_str()),  1.0f);
                glUniform1f(glGetUniformLocation(prog, (base+".linear").c_str()),    0.027f);
                glUniform1f(glGetUniformLocation(prog, (base+".quadratic").c_str()), 0.0028f);
                setUniformBool(prog, base+".enabled", fixturesOn);
            }
        }

        // Drone spotlight 
        {
            std::string base = "spotLights[" + std::to_string(spIdx) + "]";
            glUniform3f(glGetUniformLocation(prog, (base+".position").c_str()),
                        droneX, droneY, droneZ);
            glUniform3f(glGetUniformLocation(prog, (base+".direction").c_str()),
                        droneFwdX, droneFwdY, droneFwdZ);
            glUniform3f(glGetUniformLocation(prog, (base+".ambient").c_str()),
                        0.0f, 0.0f, 0.0f);
            glUniform3f(glGetUniformLocation(prog, (base+".diffuse").c_str()),
                        1.0f, 1.0f, 0.95f);
            glUniform3f(glGetUniformLocation(prog, (base+".specular").c_str()),
                        0.80f, 0.80f, 0.80f);
            glUniform1f(glGetUniformLocation(prog, (base+".cutOffInner").c_str()),
                        std::cos(0.175f));   // 10°
            glUniform1f(glGetUniformLocation(prog, (base+".cutOffOuter").c_str()),
                        std::cos(0.349f));   // 20°
            glUniform1f(glGetUniformLocation(prog, (base+".constant").c_str()),  1.0f);
            glUniform1f(glGetUniformLocation(prog, (base+".linear").c_str()),    0.045f);
            glUniform1f(glGetUniformLocation(prog, (base+".quadratic").c_str()), 0.0075f);
            setUniformBool(prog, base+".enabled", app.droneLights);
            spIdx++;
        }

        setUniform1i(prog, "numSpotLights", spIdx);

        // Default material (vertex colour as diffuse, low shininess)
        setUniformBool(prog, "useMaterialOverride", false);
        glUniform3f(glGetUniformLocation(prog, "material.ambient"),  0.2f, 0.2f, 0.2f);
        glUniform3f(glGetUniformLocation(prog, "material.specular"), 0.3f, 0.3f, 0.3f);
        glUniform1f(glGetUniformLocation(prog, "material.shininess"), 16.0f);
    }
};

#endif // LIGHTING_H
