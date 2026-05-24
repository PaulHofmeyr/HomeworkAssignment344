#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "shader.hpp"
#include "Transformations.h"
#include "AppState.h"
#include "Drone.h"
#include "PostProcess.h"
#include "Lighting.h"
#include "SceneRoot.h"   // <-- replaces Scene.h; owns the full scene graph

using namespace std;

static const char *getError()
{
    const char *desc;
    glfwGetError(&desc);
    return desc ? desc : "unknown error";
}

static GLFWwindow *createWindow(int w, int h, const char *title)
{
    glewExperimental = GL_TRUE;
    if (!glfwInit()) { cerr << "glfwInit failed: " << getError() << "\n"; exit(1); }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *win = glfwCreateWindow(w, h, title, nullptr, nullptr);
    if (!win) { cerr << "glfwCreateWindow failed: " << getError() << "\n"; glfwTerminate(); exit(1); }
    glfwMakeContextCurrent(win);

    GLenum err = glewInit();
    if (err != GLEW_OK && err != GLEW_ERROR_NO_GLX_DISPLAY)
    { cerr << "glewInit: " << glewGetErrorString(err) << "\n"; glfwTerminate(); exit(1); }
    return win;
}

static void setMat4(GLuint prog, const char *name, const Matrix<4,4> &m)
{
    float flat[16];
    flattenMatrix4(m, flat);
    glUniformMatrix4fv(glGetUniformLocation(prog, name), 1, GL_FALSE, flat);
}

int main()
{
    AppState &app = AppState::get();
    app.windowWidth  = 1000;
    app.windowHeight = 1000;

    GLFWwindow *window = createWindow(app.windowWidth, app.windowHeight,
                                      "COS344 – Crescent Head Mini-Golf");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.53f, 0.81f, 0.98f, 1.0f);

    app.sceneShaderID = LoadShaders("vertexShader.glsl", "fragmentShader.glsl");
    app.ppShaderID    = LoadShaders("postProcessVert.glsl", "postProcessFrag.glsl");

    PostProcess pp;
    pp.init(app.windowWidth, app.windowHeight);

    // ── Build the full scene graph ───────────────────────────
    buildSceneRoot();

    // ── Drone – start high so the full course is visible ────
    Drone drone;
    drone.px  =  0.0f;
    drone.py  = 40.0f;
    drone.pz  = 55.0f;
    drone.yaw =  0.0f;

    Lighting lighting;

    float orthoH = 32.0f;
    float orthoW = orthoH * ((float)app.windowWidth / app.windowHeight);
    Matrix<4,4> orthoMatrix = makeOrthographic(-orthoW, orthoW, -orthoH, orthoH, 0.1f, 300.0f);

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // ── Rotor state ──────────────────────────────────────────
    static Matrix<4,4> rotorSpin = getIdentity4();
    static bool   rotorOn   = true;
    static double lastR     = 0.0;
    static double lastEnter = 0.0;

    // Rotor pivot point (local to windmill geometry)
    static const float pivX = 0.0f, pivY = 1.48f, pivZ = 0.43f;

    do
    {
        glfwPollEvents();
        drone.processInput(window);

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) break;

        // Wireframe toggle (Enter)
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
            double now = glfwGetTime();
            if (now - lastEnter > 0.3) { app.wireframe = !app.wireframe; lastEnter = now; }
        }

        // Rotor on/off (R)
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            double now2 = glfwGetTime();
            if (now2 - lastR > 0.3) { rotorOn = !rotorOn; lastR = now2; }
        }

        // Advance rotor spin
        if (rotorOn)
            rotorSpin = makeArbitraryRotation(0.03f, 0.f, 0.f, 1.f) * rotorSpin;

        // Push spin into the windmill rotor SceneNode
        Matrix<4,4> toPiv  = makeTranslation3D(-pivX, -pivY, -pivZ);
        Matrix<4,4> frmPiv = makeTranslation3D( pivX,  pivY,  pivZ);
        updateRotor(frmPiv * rotorSpin * toPiv);

        // Matrices
        Matrix<4,4> perspMatrix = makePerspective(
            drone.fovY, (float)app.windowWidth / app.windowHeight, 0.1f, 300.0f);
        Matrix<4,4> view = drone.viewMatrix();
        Matrix<4,4> proj = app.perspProj ? perspMatrix : orthoMatrix;

        float dx, dy, dz, dfx, dfy, dfz;
        drone.getPosition(dx, dy, dz);
        drone.getForward(dfx, dfy, dfz);

        // ── Render to FBO ────────────────────────────────────
        pp.bindFBO();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(app.sceneShaderID);

        glUniform3f(glGetUniformLocation(app.sceneShaderID, "viewPos"), dx, dy, dz);
        setMat4(app.sceneShaderID, "viewMatrix",       view);
        setMat4(app.sceneShaderID, "projectionMatrix", proj);
        lighting.upload(app.sceneShaderID, app, dx, dy, dz, dfx, dfy, dfz);

        // ── Single call draws the entire scene graph ─────────
        drawSceneRoot(app.sceneShaderID, app.wireframe);

        pp.unbindFBO();
        glClear(GL_COLOR_BUFFER_BIT);
        pp.draw(app.ppShaderID);
        glfwSwapBuffers(window);

    } while (!glfwWindowShouldClose(window));

    cleanupSceneRoot();
    glDeleteProgram(app.sceneShaderID);
    glDeleteProgram(app.ppShaderID);
    glfwTerminate();
    return 0;
}
