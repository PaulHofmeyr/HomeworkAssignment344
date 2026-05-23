#ifndef APPSTATE_H
#define APPSTATE_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Transformations.h"

// TimeState for day/night cycle

enum class TimeState
{
    MIDDAY,
    DUSK,
    NIGHT
};

// Post-processing modes

enum class PostMode
{
    NONE,
    NIGHTVISION,
    GREYSCALE,
    INVERTED,
    MONOCHROME,
    FISHEYE
};

// AppState global singleton

class AppState
{
public:
    static AppState &get()
    {
        static AppState instance;
        return instance;
    }

    // Window
    int windowWidth = 1000;
    int windowHeight = 1000;

    // Rendering
    bool wireframe = false;
    TimeState timeState = TimeState::MIDDAY;
    PostMode postMode = PostMode::NONE;
    bool droneLights = false;
    bool perspProj = true;

    // Active shaders
    GLuint sceneShaderID = 0;
    GLuint ppShaderID = 0;

    // Camera / view
    Matrix<4, 4> viewMatrix = getIdentity4();
    Matrix<4, 4> projMatrix = getIdentity4();

private:
    AppState() = default;
    AppState(const AppState &) = delete;
    AppState &operator=(const AppState &) = delete;
};

#endif // APPSTATE_H
