#ifndef GLBSHAPE_H
#define GLBSHAPE_H

#include "Shape.h"
#include "GlbMesh.h"
#include <GL/glew.h>

// Wraps a header-only GlbMesh asset so it can live in the scene graph as Shape*.
class GlbShape : public Shape
{
public:
    explicit GlbShape(GlbMesh* asset)
        : Shape(1.f, 1.f, 1.f), asset_(asset) {}

    ~GlbShape() override { delete asset_; }

    GlbMesh& mesh() const { return *asset_; }

    void build() override {}

    void drawFilled() const override
    {
        GLint prog = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
        if (prog <= 0 || !asset_->isLoaded()) return;

        if (glGetUniformLocation((GLuint)prog, "lightSpaceMatrix") >= 0 &&
            glGetUniformLocation((GLuint)prog, "material.ambient") < 0)
            asset_->drawDepth((GLuint)prog);
        else
            asset_->draw((GLuint)prog, false);
    }

    void drawWireframe() const override
    {
        GLint prog = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
        if (prog > 0 && asset_->isLoaded())
            asset_->draw((GLuint)prog, true);
    }

private:
    GlbMesh* asset_;
};

template<typename Asset, typename... Args>
inline GlbShape* makeGlbShape(Args&&... args)
{
    return new GlbShape(new Asset(std::forward<Args>(args)...));
}

#endif // GLBSHAPE_H
