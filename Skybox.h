#ifndef SKYBOX_H
#define SKYBOX_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <array>
#include <cstdio>

// stb_image is used for loading the face images.
// Define STB_IMAGE_IMPLEMENTATION in exactly one .cpp file before including
// this header for the first time, OR add stb_image.cpp to your build.
// The simplest approach: put  #define STB_IMAGE_IMPLEMENTATION  in Skybox.cpp.
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "AppState.h"
#include "Transformations.h"

// ─────────────────────────────────────────────────────────────────────────────
// Skybox
//
// Usage:
//   Skybox sky;
//   sky.init();                        // call once after GL context is ready
//   sky.draw(skyboxShaderID, view, proj);  // call each frame BEFORE drawScene()
// ─────────────────────────────────────────────────────────────────────────────

class Skybox
{
public:
    Skybox()  = default;
    ~Skybox() { cleanup(); }

    // ── Load both cubemaps and build the VAO ──────────────────────────────────
    void init()
    {
        // ── Day faces (skybox-day/) ───────────────────────────────────────────
        // IMPORTANT: OpenGL cubemap face order is
        //   +X, -X, +Y, -Y, +Z, -Z
        // Map your filenames to that order:
        //   right  = +X,  left   = -X
        //   top    = +Y,  bottom = -Y
        //   front  = +Z,  back   = -Z
        std::array<std::string, 6> dayFaces = {
            "skybox-day/right.jpg",   // +X
            "skybox-day/left.jpg",    // -X
            "skybox-day/top.jpg",     // +Y
            "skybox-day/bottom.jpg",  // -Y
            "skybox-day/front.jpg",   // +Z
            "skybox-day/back.jpg"     // -Z
        };

        // ── Night faces (skybox-night/) ───────────────────────────────────────
        //   posx = +X,  negx = -X
        //   posy = +Y,  negy = -Y
        //   posz = +Z,  negz = -Z
        std::array<std::string, 6> nightFaces = {
            "skybox-night/posx.jpg",  // +X
            "skybox-night/negx.jpg",  // -X
            "skybox-night/posy.jpg",  // +Y
            "skybox-night/negy.jpg",  // -Y
            "skybox-night/posz.jpg",  // +Z
            "skybox-night/negz.jpg"   // -Z
        };

        dayTex   = _loadCubemap(dayFaces);
        nightTex = _loadCubemap(nightFaces);

        _buildVAO();
    }

    // ── Draw the skybox ───────────────────────────────────────────────────────
    // Call this BEFORE drawing the rest of the scene so the box sits behind
    // everything.  Depth writing must be re-enabled afterwards (it is).
    void draw(GLuint shader,
              const Matrix<4,4>& view,
              const Matrix<4,4>& proj) const
    {
        if (!vao || !shader) return;

        // Choose the correct cubemap based on current time state
        const AppState& app = AppState::get();
        GLuint tex = (app.timeState == TimeState::NIGHT) ? nightTex : dayTex;

        // Render with depth testing but NOT depth writing so the skybox
        // always appears behind every other object.
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);   // pass when depth == 1.0 (far plane, set by xyww trick)

        glUseProgram(shader);

        // Upload matrices
        float flatV[16], flatP[16];
        flattenMatrix4(view, flatV);
        flattenMatrix4(proj, flatP);
        glUniformMatrix4fv(glGetUniformLocation(shader, "viewMatrix"),       1, GL_FALSE, flatV);
        glUniformMatrix4fv(glGetUniformLocation(shader, "projectionMatrix"), 1, GL_FALSE, flatP);

        // Bind the cubemap to texture unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
        glUniform1i(glGetUniformLocation(shader, "skybox"), 0);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        // Restore state
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
    }

    void cleanup()
    {
        if (vao) { glDeleteVertexArrays(1, &vao); vao = 0; }
        if (vbo) { glDeleteBuffers(1, &vbo);       vbo = 0; }
        if (dayTex)   { glDeleteTextures(1, &dayTex);   dayTex   = 0; }
        if (nightTex) { glDeleteTextures(1, &nightTex); nightTex = 0; }
    }

private:
    GLuint vao = 0, vbo = 0;
    GLuint dayTex = 0, nightTex = 0;

    // ── Load a cubemap from 6 image paths ─────────────────────────────────────
    GLuint _loadCubemap(const std::array<std::string,6>& faces) const
    {
        GLuint texID;
        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

        stbi_set_flip_vertically_on_load(false);  // cubemap faces must NOT be flipped

        for (int i = 0; i < 6; i++) {
            int w, h, channels;
            unsigned char* data = stbi_load(faces[i].c_str(), &w, &h, &channels, 0);
            if (data) {
                GLenum fmt = (channels == 4) ? GL_RGBA : GL_RGB;
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                             0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
            } else {
                printf("[Skybox] Failed to load face: %s\n", faces[i].c_str());
                stbi_image_free(data);
            }
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        return texID;
    }

    // ── Build the unit cube VAO ───────────────────────────────────────────────
    void _buildVAO()
    {
        // A unit cube centred at the origin — 36 vertices (6 faces × 2 triangles × 3 vertices).
        // The positions are also used directly as the cubemap direction vectors.
        static const float verts[] = {
            // +X face
            1,-1,-1,  1,-1, 1,  1, 1, 1,
            1, 1, 1,  1, 1,-1,  1,-1,-1,
            // -X face
           -1,-1, 1, -1,-1,-1, -1, 1,-1,
           -1, 1,-1, -1, 1, 1, -1,-1, 1,
            // +Y face
           -1, 1,-1,  1, 1,-1,  1, 1, 1,
            1, 1, 1, -1, 1, 1, -1, 1,-1,
            // -Y face
           -1,-1, 1,  1,-1, 1,  1,-1,-1,
            1,-1,-1, -1,-1,-1, -1,-1, 1,
            // +Z face
           -1,-1, 1, -1, 1, 1,  1, 1, 1,
            1, 1, 1,  1,-1, 1, -1,-1, 1,
            // -Z face
            1,-1,-1,  1, 1,-1, -1, 1,-1,
           -1, 1,-1, -1,-1,-1,  1,-1,-1
        };

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }
};

#endif // SKYBOX_H
