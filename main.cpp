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
#include "SceneRoot.h"
#include "Skybox.h"

using namespace std;

static const char *getError()
{
    const char *desc; glfwGetError(&desc);
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
    float flat[16]; flattenMatrix4(m, flat);
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
    app.sceneShaderID    = LoadShaders("vertexShader.glsl",    "fragmentShader.glsl");
    app.ppShaderID       = LoadShaders("postProcessVert.glsl", "postProcessFrag.glsl");
    GLuint skyboxShader  = LoadShaders("skyboxVert.glsl", "skyboxFrag.glsl");
    GLuint sunDepthShader  = LoadShaders("shadowDepthVert.glsl", "shadowDepthFrag.glsl");
    GLuint spotDepthShader = LoadShaders("spotDepthVert.glsl",   "spotDepthFrag.glsl");
    GLuint cubeDepthShader = LoadShaders("pointCubeDepthVert.glsl",
                                         "pointCubeDepthGeom.glsl",
                                         "pointCubeDepthFrag.glsl");

    // ── Sun shadow map FBO ────────────────────────────────────
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
    float border[] = {1,1,1,1};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
    glBindFramebuffer(GL_FRAMEBUFFER, sunFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, sunDepthTex, 0);
    glDrawBuffer(GL_NONE); glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ── Spotlight shadow map array ────────────────────────────
    GLuint spotArrayTex, spotFBO;
    glGenTextures(1, &spotArrayTex);
    glBindTexture(GL_TEXTURE_2D_ARRAY, spotArrayTex);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F,
                 SHADOW_W, SHADOW_W, MAX_SPOT_SHADOWS,
                 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, border);
    glGenFramebuffers(1, &spotFBO);

    // ── Point-light cube shadow maps ──────────────────────────────────────────
    static constexpr int MAX_POINT_CUBE_SHADOWS_MAIN = 4;
    GLuint cubeShadowTex[MAX_POINT_CUBE_SHADOWS_MAIN];
    GLuint cubeFBO[MAX_POINT_CUBE_SHADOWS_MAIN];
    glGenTextures(MAX_POINT_CUBE_SHADOWS_MAIN, cubeShadowTex);
    glGenFramebuffers(MAX_POINT_CUBE_SHADOWS_MAIN, cubeFBO);
    for (int ci = 0; ci < MAX_POINT_CUBE_SHADOWS_MAIN; ci++) {
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeShadowTex[ci]);
        for (int f = 0; f < 6; f++)
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, 0,
                         GL_DEPTH_COMPONENT32F, SHADOW_W, SHADOW_W,
                         0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glBindFramebuffer(GL_FRAMEBUFFER, cubeFBO[ci]);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, cubeShadowTex[ci], 0);
        glDrawBuffer(GL_NONE); glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // ── Post-process FBO + Skybox ─────────────────────────────────────────────
    PostProcess pp;
    pp.init(app.windowWidth, app.windowHeight);
    Skybox skybox;
    skybox.init();

    // ── Build full scene graph ────────────────────────────────
    buildSceneRoot();

    // ── Drone ─────────────────────────────────────────────────
    Drone drone;
    drone.px  =  0.0f;
    drone.py  = 40.0f;
    drone.pz  = 55.0f;
    drone.yaw =  0.0f;

    Lighting lighting;

    float orthoH = 32.0f;
    float orthoW = orthoH * ((float)app.windowWidth / app.windowHeight);
    Matrix<4,4> orthoMatrix = makeOrthographic(-orthoW, orthoW, -orthoH, orthoH, 0.1f, 300.0f);

    Matrix<4,4> sunLightProj = makeOrthographic(-12.f, 12.f, -12.f, 12.f, 1.0f, 40.f);
    Matrix<4,4> sunLightView = makeLookAt(-5.f, 15.f, -3.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);
    Matrix<4,4> sunLSM = sunLightProj * sunLightView;

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // ── Rotor state ───────────────────────────────────────────
    static Matrix<4,4> rotorSpin = getIdentity4();
    static bool   rotorOn   = true;
    static double lastR     = 0.0;
    static double lastEnter = 0.0;
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

        if (rotorOn)
            rotorSpin = makeArbitraryRotation(0.03f, 0.f, 0.f, 1.f) * rotorSpin;

        // Push spin into windmill rotor SceneNode
        Matrix<4,4> toPiv  = makeTranslation3D(-pivX, -pivY, -pivZ);
        Matrix<4,4> frmPiv = makeTranslation3D( pivX,  pivY,  pivZ);
        Matrix<4,4> rotorModel = frmPiv * rotorSpin * toPiv;
        updateRotor(rotorModel);

        float dx, dy, dz, dfx, dfy, dfz;
        drone.getPosition(dx, dy, dz);
        drone.getForward(dfx, dfy, dfz);

        // Compute lighting (needed before depth passes)
        lighting.upload(app.sceneShaderID, app, dx, dy, dz, dfx, dfy, dfz);

        // ── 1. Sun shadow depth pass ──────────────────────────
        glViewport(0, 0, SHADOW_W, SHADOW_W);
        glBindFramebuffer(GL_FRAMEBUFFER, sunFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glUseProgram(sunDepthShader);
        setMat4(sunDepthShader, "lightSpaceMatrix", sunLSM);
        { float id[16]; flattenMatrix4(getIdentity4(), id);
          glUniformMatrix4fv(glGetUniformLocation(sunDepthShader, "modelMatrix"), 1, GL_FALSE, id); }
        drawSceneRoot(sunDepthShader, false);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // ── 2. Spotlight shadow depth passes ─────────────────
        glViewport(0, 0, SHADOW_W, SHADOW_W);
        glUseProgram(spotDepthShader);
        for (int i = 0; i < lighting.numSpotShadows; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, spotFBO);
            glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                      spotArrayTex, 0, i);
            glDrawBuffer(GL_NONE); glReadBuffer(GL_NONE);
            glClear(GL_DEPTH_BUFFER_BIT);
            setMat4(spotDepthShader, "lightSpaceMatrix", lighting.spotLSMs[i]);
            { float id[16]; flattenMatrix4(getIdentity4(), id);
              glUniformMatrix4fv(glGetUniformLocation(spotDepthShader, "modelMatrix"), 1, GL_FALSE, id); }
            drawSceneRoot(spotDepthShader, false);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // ── 3b. Point-light cube shadow passes (6 faces × N lights) ───────────
        glViewport(0, 0, SHADOW_W, SHADOW_W);
        glUseProgram(cubeDepthShader);
        for (int ci = 0; ci < lighting.numPointCubeShadows; ci++) {
            glBindFramebuffer(GL_FRAMEBUFFER, cubeFBO[ci]);
            glClear(GL_DEPTH_BUFFER_BIT);

            for (int f = 0; f < 6; f++) {
                std::string uname = "shadowMatrices[" + std::to_string(f) + "]";
                setMat4(cubeDepthShader, uname.c_str(), lighting.pointCubeLSMs[ci].m[f]);
            }
            float lx = lighting.bollardPos[ci].x;
            float ly = lighting.bollardPos[ci].y;
            float lz = lighting.bollardPos[ci].z;
            glUniform3f(glGetUniformLocation(cubeDepthShader, "lightPos"), lx, ly, lz);
            glUniform1f(glGetUniformLocation(cubeDepthShader, "farPlane"), lighting.pointShadowFarPlane);

            { float id[16]; flattenMatrix4(getIdentity4(), id);
              glUniformMatrix4fv(glGetUniformLocation(cubeDepthShader, "modelMatrix"), 1, GL_FALSE, id); }
            drawSceneRoot(cubeDepthShader, false);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // ── 3. Scene pass → post-process FBO ─────────────────
        glViewport(0, 0, app.windowWidth, app.windowHeight);

        Matrix<4,4> perspMatrix = makePerspective(
            drone.fovY, (float)app.windowWidth / app.windowHeight, 0.1f, 300.0f);
        Matrix<4,4> view = drone.viewMatrix();
        Matrix<4,4> proj = app.perspProj ? perspMatrix : orthoMatrix;

        pp.bindFBO();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw skybox first (behind everything). skybox.draw will set its own
        // shader as active, so restore the scene shader afterwards.
        skybox.draw(skyboxShader, view, proj);

        glUseProgram(app.sceneShaderID);

        glUniform3f(glGetUniformLocation(app.sceneShaderID, "viewPos"), dx, dy, dz);
        setMat4(app.sceneShaderID, "viewMatrix",       view);
        setMat4(app.sceneShaderID, "projectionMatrix", proj);

        lighting.upload(app.sceneShaderID, app, dx, dy, dz, dfx, dfy, dfz);

        // Sun shadow map → texture unit 1
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, sunDepthTex);
        glUniform1i(glGetUniformLocation(app.sceneShaderID, "sunShadowMap"), 1);
        glUniform1i(glGetUniformLocation(app.sceneShaderID, "useSunShadow"), 1);
        setMat4(app.sceneShaderID, "sunLightSpaceMatrix", sunLSM);

        // Spotlight shadow array → texture unit 2
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D_ARRAY, spotArrayTex);
        glUniform1i(glGetUniformLocation(app.sceneShaderID, "spotShadowMaps"), 2);
        glUniform1i(glGetUniformLocation(app.sceneShaderID, "useSpotShadows"), 1);
        glUniform1i(glGetUniformLocation(app.sceneShaderID, "numSpotShadows"),
                    lighting.numSpotShadows);

        // Cube shadow maps → texture units 3-6
        for (int ci = 0; ci < MAX_POINT_CUBE_SHADOWS_MAIN; ci++) {
            glActiveTexture(GL_TEXTURE3 + ci);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubeShadowTex[ci]);
            std::string uname = "pointShadowCubeMap[" + std::to_string(ci) + "]";
            glUniform1i(glGetUniformLocation(app.sceneShaderID, uname.c_str()), 3 + ci);
        }
        glUniform1i(glGetUniformLocation(app.sceneShaderID, "usePointCubeShadows"), 1);
        glUniform1i(glGetUniformLocation(app.sceneShaderID, "numPointCubeShadows"),
                    lighting.numPointCubeShadows);
        glUniform1f(glGetUniformLocation(app.sceneShaderID, "pointShadowFarPlane"),
                    lighting.pointShadowFarPlane);


        for (int i = 0; i < lighting.numSpotShadows; i++) {
            std::string uname = "spotLightSpaceMatrix[" + std::to_string(i) + "]";
            setMat4(app.sceneShaderID, uname.c_str(), lighting.spotLSMs[i]);
        }

        // ── Single call draws the entire scene graph ──────────
        drawSceneRoot(app.sceneShaderID, app.wireframe);

        pp.unbindFBO();

        // ── 4. Post-process pass ──────────────────────────────
        glClear(GL_COLOR_BUFFER_BIT);
        pp.draw(app.ppShaderID);
        glfwSwapBuffers(window);

    } while (!glfwWindowShouldClose(window));

    // ── Cleanup ───────────────────────────────────────────────
    glDeleteFramebuffers(1, &sunFBO);
    glDeleteTextures(1, &sunDepthTex);
    glDeleteFramebuffers(1, &spotFBO);
    glDeleteTextures(1, &spotArrayTex);
    glDeleteFramebuffers(MAX_POINT_CUBE_SHADOWS_MAIN, cubeFBO);
    glDeleteTextures(MAX_POINT_CUBE_SHADOWS_MAIN, cubeShadowTex);
    // Skybox cleanup
    skybox.cleanup();
    glDeleteProgram(skyboxShader);
    glDeleteProgram(sunDepthShader);
    glDeleteProgram(spotDepthShader);
    glDeleteProgram(cubeDepthShader);
    cleanupSceneRoot();
    glDeleteProgram(app.sceneShaderID);
    glDeleteProgram(app.ppShaderID);
    glfwTerminate();
    return 0;
}