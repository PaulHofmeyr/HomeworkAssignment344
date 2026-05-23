#ifndef POSTPROCESS_H
#define POSTPROCESS_H

#include <GL/glew.h>
#include "AppState.h"

// PostProcess – manages the off-screen FBO and the full-screen quad draw.
//
// Usage (each frame):
//   pp.bindFBO();          // redirect scene render to texture
//   ... draw scene ...
//   pp.unbindFBO();        // restore default framebuffer
//   pp.draw(ppShaderID);   // draw full-screen quad with post effects

class PostProcess
{
public:
    PostProcess() = default;
    ~PostProcess() { cleanup(); }

    // Call once after GL context is ready
    void init(int width, int height)
    {
        w = width;
        h = height;

        // Framebuffer
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        // Colour attachment
        glGenTextures(1, &texColour);
        glBindTexture(GL_TEXTURE_2D, texColour);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColour, 0);

        // Depth + stencil renderbuffer
        glGenRenderbuffers(1, &rboDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Full-screen quad
        static const float quadVerts[] = {
            // pos        uv
            -1.f,
            -1.f,
            0.f,
            0.f,
            1.f,
            -1.f,
            1.f,
            0.f,
            -1.f,
            1.f,
            0.f,
            1.f,
            1.f,
            1.f,
            1.f,
            1.f,
        };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }

    void bindFBO()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void unbindFBO()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Draw the result with post-processing effects
    void draw(GLuint ppShader)
    {
        AppState &app = AppState::get();
        glDisable(GL_DEPTH_TEST);
        glUseProgram(ppShader);

        // Set mode uniforms
        bool nv = (app.postMode == PostMode::NIGHTVISION);
        bool fe = (app.postMode == PostMode::FISHEYE);
        bool gs = (app.postMode == PostMode::GREYSCALE);
        bool inv = (app.postMode == PostMode::INVERTED);
        bool mono = (app.postMode == PostMode::MONOCHROME);

        glUniform1i(glGetUniformLocation(ppShader, "nightVision"), nv);
        glUniform1i(glGetUniformLocation(ppShader, "fisheye"), fe);
        glUniform1i(glGetUniformLocation(ppShader, "greyscale"), gs);
        glUniform1i(glGetUniformLocation(ppShader, "inverted"), inv);
        glUniform1i(glGetUniformLocation(ppShader, "monochrome"), mono);
        // Warm amber mono hue
        glUniform3f(glGetUniformLocation(ppShader, "monoHue"), 1.0f, 0.6f, 0.1f);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texColour);
        glUniform1i(glGetUniformLocation(ppShader, "screenTexture"), 0);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);

        glEnable(GL_DEPTH_TEST);
    }

    void cleanup()
    {
        if (fbo)
            glDeleteFramebuffers(1, &fbo);
        if (texColour)
            glDeleteTextures(1, &texColour);
        if (rboDepth)
            glDeleteRenderbuffers(1, &rboDepth);
        if (quadVAO)
            glDeleteVertexArrays(1, &quadVAO);
        if (quadVBO)
            glDeleteBuffers(1, &quadVBO);
        fbo = texColour = rboDepth = quadVAO = quadVBO = 0;
    }

private:
    int w = 0, h = 0;
    GLuint fbo = 0, texColour = 0, rboDepth = 0;
    GLuint quadVAO = 0, quadVBO = 0;
};

#endif // POSTPROCESS_H
