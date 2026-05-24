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
    { cerr << "glewInit failed: " << glewGetErrorString(err) << "\n"; glfwTerminate(); exit(1); }
    return win;
}

static void setMat4(GLuint prog, const char *name, const Matrix<4,4> &m)
{
    float flat[16];
    flattenMatrix4(m, flat);
    glUniformMatrix4fv(glGetUniformLocation(prog, name), 1, GL_FALSE, flat);
}

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

    // ── Shaders ───────────────────────────────────────────────────────────────
    app.sceneShaderID = LoadShaders("vertexShader.glsl",    "fragmentShader.glsl");
    app.ppShaderID    = LoadShaders("postProcessVert.glsl", "postProcessFrag.glsl");
    GLuint depthShaderID    = LoadShaders("shadowDepthVert.glsl", "shadowDepthFrag.glsl");
    GLuint spotDepthShaderID = LoadShaders("spotDepthVert.glsl",  "spotDepthFrag.glsl");

    // ── Sun shadow map FBO ────────────────────────────────────────────────────
    GLuint sunFBO, sunDepthTex;
    glGenFramebuffers(1, &sunFBO);
    glGenTextures(1, &sunDepthTex);
    glBindTexture(GL_TEXTURE_2D, sunDepthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 SHADOW_W, SHADOW_W, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glBindFramebuffer(GL_FRAMEBUFFER, sunFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, sunDepthTex, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ── Drone spotlight shadow map FBO ────────────────────────────────────────
    // Same structure as the sun map; re-rendered every frame because the
    // drone moves. Texture unit 2 (sun=1, post-process quad=0).
    GLuint spotFBO, spotDepthTex;
    glGenFramebuffers(1, &spotFBO);
    glGenTextures(1, &spotDepthTex);
    glBindTexture(GL_TEXTURE_2D, spotDepthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 SHADOW_W, SHADOW_W, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glBindFramebuffer(GL_FRAMEBUFFER, spotFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, spotDepthTex, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ── Other setup ───────────────────────────────────────────────────────────
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
    Matrix<4,4> orthoMatrix = makeOrthographic(-orthoW, orthoW, -orthoH, orthoH, 0.1f, 200.0f);

    // Sun light-space matrix (constant)
    Matrix<4,4> sunLightProj = makeOrthographic(-12.f, 12.f, -12.f, 12.f, 1.0f, 40.f);
    Matrix<4,4> sunLightView = makeLookAt(-5.f, 15.f, -3.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);
    Matrix<4,4> sunLSM = sunLightProj * sunLightView;

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    static Matrix<4,4> rotorSpin  = getIdentity4();
    static bool   rotorOn   = true;
    static double lastR     = 0.0;
    static double lastEnter = 0.0;

    do
    {
        glfwPollEvents();
        drone.processInput(window);

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) break;

        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
            double now = glfwGetTime();
            if (now - lastEnter > 0.3) { app.wireframe = !app.wireframe; lastEnter = now; }
        }
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            double now2 = glfwGetTime();
            if (now2 - lastR > 0.3) { rotorOn = !rotorOn; lastR = now2; }
        }
        if (rotorOn)
            rotorSpin = makeArbitraryRotation(0.03f, 0.f, 0.f, 1.f) * rotorSpin;

        static const float pivX = 0.0f, pivY = 1.48f, pivZ = 0.43f;
        Matrix<4,4> toPiv    = makeTranslation3D(-pivX, -pivY, -pivZ);
        Matrix<4,4> frmPiv   = makeTranslation3D( pivX,  pivY,  pivZ);
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
        {
            float id[16]; flattenMatrix4(getIdentity4(), id);
            glUniformMatrix4fv(glGetUniformLocation(depthShaderID, "modelMatrix"), 1, GL_FALSE, id);
        }
        drawScene(false);
        glUniformMatrix4fv(glGetUniformLocation(depthShaderID, "modelMatrix"), 1, GL_FALSE, rotorFlat);
        drawRotor(false);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // ── 2. Drone spotlight shadow depth pass ──────────────────────────────
        // upload() must run before this so droneLSM is current for this frame.
        // We call a lightweight version: just compute the LSM from drone state,
        // no need to upload scene uniforms yet.
        {
            float fLen = std::sqrt(dfx*dfx + dfy*dfy + dfz*dfz);
            if (fLen < 1e-5f) fLen = 1.0f;
            float nfx = dfx/fLen, nfy = dfy/fLen, nfz = dfz/fLen;
            float upX = 0.f, upY = 1.f, upZ = 0.f;
            if (std::fabs(nfy) > 0.99f) { upY = 0.f; upZ = 1.f; }
            Matrix<4,4> lView = makeLookAt(dx, dy, dz,
                                           dx+nfx, dy+nfy, dz+nfz,
                                           upX, upY, upZ);
            Matrix<4,4> lProj = makePerspective(0.349f * 2.0f + 0.1f, 1.0f, 0.1f, 50.0f);
            lighting.droneLSM = lProj * lView;
        }

        glViewport(0, 0, SHADOW_W, SHADOW_W);
        glBindFramebuffer(GL_FRAMEBUFFER, spotFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glUseProgram(spotDepthShaderID);
        setMat4(spotDepthShaderID, "lightSpaceMatrix", lighting.droneLSM);
        {
            float id[16]; flattenMatrix4(getIdentity4(), id);
            glUniformMatrix4fv(glGetUniformLocation(spotDepthShaderID, "modelMatrix"), 1, GL_FALSE, id);
        }
        drawScene(false);
        glUniformMatrix4fv(glGetUniformLocation(spotDepthShaderID, "modelMatrix"), 1, GL_FALSE, rotorFlat);
        drawRotor(false);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // ── 3. Scene pass → post-process FBO ─────────────────────────────────
        glViewport(0, 0, app.windowWidth, app.windowHeight);

        Matrix<4,4> perspMatrix = makePerspective(drone.fovY,
            (float)app.windowWidth / app.windowHeight, 0.1f, 200.0f);
        Matrix<4,4> view = drone.viewMatrix();
        Matrix<4,4> proj = app.perspProj ? perspMatrix : orthoMatrix;

        pp.bindFBO();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(app.sceneShaderID);

        glUniform3f(glGetUniformLocation(app.sceneShaderID, "viewPos"), dx, dy, dz);
        setMat4(app.sceneShaderID, "viewMatrix",       view);
        setMat4(app.sceneShaderID, "projectionMatrix", proj);

        lighting.upload(app.sceneShaderID, app, dx, dy, dz, dfx, dfy, dfz);

        // Bind sun shadow map → texture unit 1
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, sunDepthTex);
        glUniform1i(glGetUniformLocation(app.sceneShaderID, "sunShadowMap"), 1);
        glUniform1i(glGetUniformLocation(app.sceneShaderID, "useSunShadow"), 1);
        setMat4(app.sceneShaderID, "sunLightSpaceMatrix", sunLSM);

        // Bind drone spotlight shadow map → texture unit 2
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, spotDepthTex);
        glUniform1i(glGetUniformLocation(app.sceneShaderID, "spotShadowMap"), 2);
        // Only actually cast shadows when the drone light is on
        glUniform1i(glGetUniformLocation(app.sceneShaderID, "useSpotShadow"),
                    app.droneLights ? 1 : 0);
        setMat4(app.sceneShaderID, "spotLightSpaceMatrix", lighting.droneLSM);

        // Draw static scene
        {
            float id[16]; flattenMatrix4(getIdentity4(), id);
            glUniformMatrix4fv(glGetUniformLocation(app.sceneShaderID, "modelMatrix"), 1, GL_FALSE, id);
        }
        drawScene(app.wireframe);

        // Draw rotor
        glUniformMatrix4fv(glGetUniformLocation(app.sceneShaderID, "modelMatrix"), 1, GL_FALSE, rotorFlat);
        drawRotor(app.wireframe);

        pp.unbindFBO();

        // ── 4. Post-process pass ──────────────────────────────────────────────
        glClear(GL_COLOR_BUFFER_BIT);
        pp.draw(app.ppShaderID);

        glfwSwapBuffers(window);

    } while (!glfwWindowShouldClose(window));

    // Cleanup
    glDeleteFramebuffers(1, &sunFBO);
    glDeleteTextures(1, &sunDepthTex);
    glDeleteFramebuffers(1, &spotFBO);
    glDeleteTextures(1, &spotDepthTex);
    glDeleteProgram(depthShaderID);
    glDeleteProgram(spotDepthShaderID);
    cleanupScene();
    glDeleteProgram(app.sceneShaderID);
    glDeleteProgram(app.ppShaderID);
    glfwTerminate();
    return 0;
}
