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
#include "Scene.h" // initScene / drawScene / drawRotor / cleanupScene

using namespace std;

// GLFW helpers 
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

// Flat-matrix upload helper 
static void setMat4(GLuint prog, const char *name, const Matrix<4, 4> &m)
{
    float flat[16];
    flattenMatrix4(m, flat);
    glUniformMatrix4fv(glGetUniformLocation(prog, name), 1, GL_FALSE, flat);
}

int main()
{
    AppState &app = AppState::get();
    app.windowWidth = 1000;
    app.windowHeight = 1000;

    GLFWwindow *window = createWindow(app.windowWidth, app.windowHeight,
                                      "COS344 – Crescent Head Mini-Golf");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.53f, 0.81f, 0.98f, 1.0f);

    // Load shaders
    app.sceneShaderID = LoadShaders("vertexShader.glsl", "fragmentShader.glsl");
    app.ppShaderID = LoadShaders("postProcessVert.glsl", "postProcessFrag.glsl");

    // Post-process FBO 
    PostProcess pp;
    pp.init(app.windowWidth, app.windowHeight);

    // Scene (from practical 3, reused as windmill / course scaffold) 
    initScene();

    // Drone 
    Drone drone;
    drone.px = 0.0f;
    drone.py = 3.5f;
    drone.pz = 6.0f;
    drone.yaw = 3.14159f; // start facing the course

    // Lighting 
    Lighting lighting;

    // Orthographic projection (static - aspect doesn't change)
    // Perspective is rebuilt each frame so zoom (fovY) takes effect.

    // Orthographic (activated by 'P')
    float orthoH = 10.0f;
    float orthoW = orthoH * ((float)app.windowWidth / app.windowHeight);
    Matrix<4, 4> orthoMatrix = makeOrthographic(-orthoW, orthoW, -orthoH, orthoH, 0.1f, 200.0f);

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Main loop
    do
    {
        // Input 
        glfwPollEvents();
        drone.processInput(window);

        // ESC = quit
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            break;

        // Wireframe toggle (Enter)
        static double lastEnter = 0.0;
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
        {
            double now = glfwGetTime();
            if (now - lastEnter > 0.3)
            {
                app.wireframe = !app.wireframe;
                lastEnter = now;
            }
        }

        // Matrices
        Matrix<4, 4> perspMatrix = makePerspective(
            drone.fovY,
            (float)app.windowWidth / app.windowHeight,
            0.1f, 200.0f);
        Matrix<4, 4> view = drone.viewMatrix();
        Matrix<4, 4> proj = app.perspProj ? perspMatrix : orthoMatrix;

        // Drone position / direction for lighting 
        float dx, dy, dz, dfx, dfy, dfz;
        drone.getPosition(dx, dy, dz);
        drone.getForward(dfx, dfy, dfz);

        // Render scene to FBO 
        pp.bindFBO();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(app.sceneShaderID);

        // Camera pos for specular
        glUniform3f(glGetUniformLocation(app.sceneShaderID, "viewPos"), dx, dy, dz);

        // View / projection
        setMat4(app.sceneShaderID, "viewMatrix", view);
        setMat4(app.sceneShaderID, "projectionMatrix", proj);

        // Lighting
        lighting.upload(app.sceneShaderID, app, dx, dy, dz, dfx, dfy, dfz);

        // Draw the scene 
        // modelMatrix = identity for the main scene
        {
            float identFlat[16];
            flattenMatrix4(getIdentity4(), identFlat);
            glUniformMatrix4fv(glGetUniformLocation(app.sceneShaderID, "modelMatrix"),
                               1, GL_FALSE, identFlat);
        }
        drawScene(app.wireframe);

        // Rotor
        static Matrix<4, 4> rotorSpin = getIdentity4();

        // R to toggle rotor
        static bool rotorOn = true;
        static double lastR = 0.0;
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        {
            double now2 = glfwGetTime();
            if (now2 - lastR > 0.3)
            {
                rotorOn = !rotorOn;
                lastR = now2;
            }
        }
        if (rotorOn)
            rotorSpin = makeArbitraryRotation(0.03f, 0.f, 0.f, 1.f) * rotorSpin;

        // Build rotor model = pivot-out * spin * pivot-in
        static const float pivX = 0.0f, pivY = 1.48f, pivZ = 0.43f;
        Matrix<4, 4> toPiv = makeTranslation3D(-pivX, -pivY, -pivZ);
        Matrix<4, 4> frmPiv = makeTranslation3D(pivX, pivY, pivZ);
        Matrix<4, 4> rotorModel = frmPiv * rotorSpin * toPiv;

        float rotorFlat[16];
        flattenMatrix4(rotorModel, rotorFlat);
        glUniformMatrix4fv(glGetUniformLocation(app.sceneShaderID, "modelMatrix"),
                           1, GL_FALSE, rotorFlat);
        drawRotor(app.wireframe);

        pp.unbindFBO();

        // Post-process pass to default FBO
        glClear(GL_COLOR_BUFFER_BIT);
        pp.draw(app.ppShaderID);

        glfwSwapBuffers(window);

    } while (!glfwWindowShouldClose(window));

    // Cleanup
    cleanupScene();
    glDeleteProgram(app.sceneShaderID);
    glDeleteProgram(app.ppShaderID);
    glfwTerminate();
    return 0;
}