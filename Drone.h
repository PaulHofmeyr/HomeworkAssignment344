#ifndef DRONE_H
#define DRONE_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include "Transformations.h"
#include "AppState.h"

class Drone
{
public:
    float px = 0.0f, py = 3.0f, pz = 6.0f;
    float yaw   = 0.0f;
    float pitch = 0.0f;
    float roll  = 0.0f;

    // Slowed down so the small scene stays navigable
    float moveSpeed = 0.15f;
    float rotSpeed  = 0.04f;
    float liftSpeed = 0.15f;

    // Zoom: field-of-view in radians (45° default), clamped 10°–90°
    float fovY = 0.785398f;   // 45°
    float zoomSpeed = 0.02f;

    // Pitch / roll clamp: keep drone right-side up (±75° = 1.309 rad)
    static constexpr float TILT_LIMIT = 1.309f;

    double lastC = 0.0, lastN = 0.0, lastT = 0.0;
    double lastP = 0.0, lastPlus = 0.0, lastMinus = 0.0;
    static constexpr double DEBOUNCE = 0.25;

    Drone() = default;

    void processInput(GLFWwindow* window)
    {
        AppState& app = AppState::get();
        double now = glfwGetTime();

        // Forward / right vectors derived from yaw only (so strafe is flat)
        float fwdX = -std::sin(yaw);
        float fwdZ = -std::cos(yaw);
        float rtX  =  std::cos(yaw);   //  right = rotate fwd +90° around Y
        float rtZ  = -std::sin(yaw);

        // W/S  – move forward / backward
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { px += fwdX * moveSpeed; pz += fwdZ * moveSpeed; }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { px -= fwdX * moveSpeed; pz -= fwdZ * moveSpeed; }

        // A/D  – strafe left / right
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { px -= rtX * moveSpeed; pz -= rtZ * moveSpeed; }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { px += rtX * moveSpeed; pz += rtZ * moveSpeed; }

        // Space / Ctrl  – up / down
        if (glfwGetKey(window, GLFW_KEY_SPACE)        == GLFW_PRESS) py += liftSpeed;
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) py -= liftSpeed;

        // Q/E  – yaw left / right
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) yaw += rotSpeed;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) yaw -= rotSpeed;

        // I/K  – pitch up / down  (clamped)
        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) pitch += rotSpeed;
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) pitch -= rotSpeed;
        if (pitch >  TILT_LIMIT) pitch =  TILT_LIMIT;
        if (pitch < -TILT_LIMIT) pitch = -TILT_LIMIT;

        // J/L  – roll left / right  (clamped)
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) roll -= rotSpeed;
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) roll += rotSpeed;
        if (roll >  TILT_LIMIT) roll =  TILT_LIMIT;
        if (roll < -TILT_LIMIT) roll = -TILT_LIMIT;

        // Z/X  – zoom in / out (decrease / increase FOV)
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) fovY -= zoomSpeed;
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) fovY += zoomSpeed;
        if (fovY < 0.1745f) fovY = 0.1745f;   // min ~10°
        if (fovY > 1.5708f) fovY = 1.5708f;   // max  90°

        // C - cycle post mode
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && now - lastC > DEBOUNCE) {
            lastC = now;
            app.postMode = (PostMode)(((int)app.postMode + 1) % 6);
        }
        // N - toggle night-vision
        if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS && now - lastN > DEBOUNCE) {
            lastN = now;
            app.postMode = (app.postMode == PostMode::NIGHTVISION) ? PostMode::NONE : PostMode::NIGHTVISION;
        }
        // T - drone spotlight
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && now - lastT > DEBOUNCE) {
            lastT = now;
            app.droneLights = !app.droneLights;
        }
        // P - perspective / ortho
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && now - lastP > DEBOUNCE) {
            lastP = now;
            app.perspProj = !app.perspProj;
        }
        // + / - time state
        if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS && now - lastPlus > DEBOUNCE) {
            lastPlus = now;
            app.timeState = (TimeState)(((int)app.timeState + 1) % 3);
        }
        if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS && now - lastMinus > DEBOUNCE) {
            lastMinus = now;
            app.timeState = (TimeState)(((int)app.timeState + 2) % 3);
        }
    }

    Matrix<4,4> viewMatrix() const
    {
        float cy = std::cos(yaw),   sy = std::sin(yaw);
        float cp = std::cos(pitch), sp = std::sin(pitch);
        float cr = std::cos(roll),  sr = std::sin(roll);

        // Forward vector
        float fwdX = -sy * cp;
        float fwdY =  sp;
        float fwdZ = -cy * cp;

        // Right vector (yaw only)
        float rtX =  cy;
        float rtZ = -sy;

        // Up = cos(roll)*worldUp + sin(roll)*right  (projected)
        float upX = sr * rtX;
        float upY = cr;
        float upZ = sr * rtZ;

        return makeLookAt(px, py, pz,
                          px + fwdX, py + fwdY, pz + fwdZ,
                          upX, upY, upZ);
    }

    void getPosition(float& x, float& y, float& z) const { x = px; y = py; z = pz; }

    void getForward(float& x, float& y, float& z) const
    {
        float cy = std::cos(yaw), sy = std::sin(yaw);
        float cp = std::cos(pitch), sp = std::sin(pitch);
        x = -sy * cp;
        y =  sp;
        z = -cy * cp;
    }
};

#endif // DRONE_H