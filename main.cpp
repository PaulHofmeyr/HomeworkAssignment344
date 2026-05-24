#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "shader.hpp"
#include "Transformations.h"
#include "AppState.h"
#include "SceneNode.h"
#include "Drone.h"
#include "PostProcess.h"
#include "Lighting.h"
#include "Scene.h"

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
    if (!glfwInit())
    {
        cerr << "glfwInit failed: " << getError() << "\n";
        exit(1);
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *win = glfwCreateWindow(w, h, title, nullptr, nullptr);
    if (!win)
    {
        cerr << "glfwCreateWindow failed: " << getError() << "\n";
        glfwTerminate();
        exit(1);
    }
    glfwMakeContextCurrent(win);

    GLenum err = glewInit();
    if (err != GLEW_OK && err != GLEW_ERROR_NO_GLX_DISPLAY)
    {
        cerr << "glewInit failed: " << glewGetErrorString(err) << "\n";
        glfwTerminate();
        exit(1);
    }
    return win;
}

static void setMat4(GLuint prog, const char *name, const Matrix<4, 4> &m)
{
    float flat[16];
    flattenMatrix4(m, flat);
    glUniformMatrix4fv(glGetUniformLocation(prog, name), 1, GL_FALSE, flat);
}

// ── Shadow map constants ──────────────────────────────────────────────────────
static const int SHADOW_W = 2048;

int main()
{
    AppState &app = AppState::get();
    app.windowWidth  = 1000;
    app.windowHeight = 1000;

    GLFWwindow *window = createWindow(app.windowWidth, app.windowHeight,
                                      "COS344 – Crescent Head Mini-Golf");

    glfwGetFramebufferSize(window, &app.windowWidth, &app.windowHeight);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.53f, 0.81f, 0.98f, 1.0f);

    // Load shaders
    app.sceneShaderID = LoadShaders("vertexShader.glsl", "fragmentShader.glsl");
    app.ppShaderID    = LoadShaders("postProcessVert.glsl", "postProcessFrag.glsl");
    GLuint depthShaderID = LoadShaders("shadowDepthVert.glsl", "shadowDepthFrag.glsl");

    // ── Sun shadow map FBO ────────────────────────────────────────────────────
    GLuint sunFBO, sunDepthTex;
    glGenFramebuffers(1, &sunFBO);
    glGenTextures(1, &sunDepthTex);
    glBindTexture(GL_TEXTURE_2D, sunDepthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 SHADOW_W, SHADOW_W, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // Fragments outside the light frustum should not be shadowed
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glBindFramebuffer(GL_FRAMEBUFFER, sunFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_2D, sunDepthTex, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Post-process FBO
    PostProcess pp;
    pp.init(app.windowWidth, app.windowHeight);

    initScene();

    Drone drone;
    drone.px  = 0.0f;
    drone.py  = 3.5f;
    drone.pz  = 6.0f;
    drone.yaw = 0.0f;

    Lighting lighting;

    float orthoH = 10.0f;
    float orthoW = orthoH * ((float)app.windowWidth / app.windowHeight);
    Matrix<4, 4> orthoMatrix = makeOrthographic(-orthoW, orthoW, -orthoH, orthoH, 0.1f, 200.0f);

    // Sun light-space matrix (constant — sun doesn't move)
    Matrix<4,4> sunLightProj = makeOrthographic(-12.f, 12.f, -12.f, 12.f, 1.0f, 40.f);
    Matrix<4,4> sunLightView = makeLookAt(-5.f, 15.f, -3.f,
                                           0.f,  0.f,  0.f,
                                           0.f,  1.f,  0.f);
    Matrix<4,4> sunLSM = sunLightProj * sunLightView;

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    static Matrix<4, 4> rotorSpin = getIdentity4();
    static bool   rotorOn  = true;
    static double lastR    = 0.0;
    static double lastEnter = 0.0;

    do
    {
        glfwPollEvents();
        drone.processInput(window);

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            break;

        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
        {
            double now = glfwGetTime();
            if (now - lastEnter > 0.3) { app.wireframe = !app.wireframe; lastEnter = now; }
        }

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        {
            double now2 = glfwGetTime();
            if (now2 - lastR > 0.3) { rotorOn = !rotorOn; lastR = now2; }
        }
        if (rotorOn)
            rotorSpin = makeArbitraryRotation(0.03f, 0.f, 0.f, 1.f) * rotorSpin;

        static const float pivX = 0.0f, pivY = 1.48f, pivZ = 0.43f;
        Matrix<4,4> toPiv     = makeTranslation3D(-pivX, -pivY, -pivZ);
        Matrix<4,4> frmPiv    = makeTranslation3D( pivX,  pivY,  pivZ);
        Matrix<4,4> rotorModel = frmPiv * rotorSpin * toPiv;

        float rotorFlat[16];
        flattenMatrix4(rotorModel, rotorFlat);

        float dx, dy, dz, dfx, dfy, dfz;
        drone.getPosition(dx, dy, dz);
        drone.getForward(dfx, dfy, dfz);

        // ── 1. Sun shadow depth pass ──────────────────────────────────────────
        glViewport(0, 0, SHADOW_W, SHADOW_W);
        glBindFramebuffer(GL_FRAMEBUFFER, sunFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glUseProgram(depthShaderID);
        setMat4(depthShaderID, "lightSpaceMatrix", sunLSM);

        // Static scene
        {
            float id[16]; flattenMatrix4(getIdentity4(), id);
            glUniformMatrix4fv(glGetUniformLocation(depthShaderID, "modelMatrix"),
                               1, GL_FALSE, id);
        }
        drawScene(false);

        // Rotor
        glUniformMatrix4fv(glGetUniformLocation(depthShaderID, "modelMatrix"),
                           1, GL_FALSE, rotorFlat);
        drawRotor(false);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // ── 2. Scene pass → post-process FBO ─────────────────────────────────
        glViewport(0, 0, app.windowWidth, app.windowHeight);

        Matrix<4,4> perspMatrix = makePerspective(
            drone.fovY,
            (float)app.windowWidth / app.windowHeight,
            0.1f, 200.0f);
        Matrix<4,4> view = drone.viewMatrix();
        Matrix<4,4> proj = app.perspProj ? perspMatrix : orthoMatrix;

        pp.bindFBO();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(app.sceneShaderID);

        glUniform3f(glGetUniformLocation(app.sceneShaderID, "viewPos"), dx, dy, dz);
        setMat4(app.sceneShaderID, "viewMatrix",       view);
        setMat4(app.sceneShaderID, "projectionMatrix", proj);

        lighting.upload(app.sceneShaderID, app, dx, dy, dz, dfx, dfy, dfz);

        // Bind sun shadow map to texture unit 1
        // (unit 0 is used by the post-process quad — keep them separate)
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, sunDepthTex);
        glUniform1i(glGetUniformLocation(app.sceneShaderID, "sunShadowMap"), 1);
        glUniform1i(glGetUniformLocation(app.sceneShaderID, "useSunShadow"), 1);
        setMat4(app.sceneShaderID, "sunLightSpaceMatrix", sunLSM);

        // Static scene
        {
            float id[16]; flattenMatrix4(getIdentity4(), id);
            glUniformMatrix4fv(glGetUniformLocation(app.sceneShaderID, "modelMatrix"),
                               1, GL_FALSE, id);
        }
        drawScene(app.wireframe);

        // Rotor
        glUniformMatrix4fv(glGetUniformLocation(app.sceneShaderID, "modelMatrix"),
                           1, GL_FALSE, rotorFlat);
        drawRotor(app.wireframe);

        pp.unbindFBO();

        // ── 3. Post-process pass ──────────────────────────────────────────────
        glClear(GL_COLOR_BUFFER_BIT);
        pp.draw(app.ppShaderID);

        glfwSwapBuffers(window);

    } while (!glfwWindowShouldClose(window));

    // Cleanup
    glDeleteFramebuffers(1, &sunFBO);
    glDeleteTextures(1, &sunDepthTex);
    glDeleteProgram(depthShaderID);
    cleanupScene();
    glDeleteProgram(app.sceneShaderID);
    glDeleteProgram(app.ppShaderID);
    glfwTerminate();
    return 0;
}