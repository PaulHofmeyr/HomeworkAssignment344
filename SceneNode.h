#ifndef SCENENODE_H
#define SCENENODE_H

#include <vector>
#include <memory>
#include "Transformations.h"
#include "Shape.h"

// SceneNode – the building block of the scene graph.
//
//   • Holds a local transform (Matrix<4,4>).
//   • Optionally owns a Shape* (the renderable leaf geometry).
//   • Can have an arbitrary number of child nodes.
//   • draw() cascades the accumulated world transform down to children.

class SceneNode
{
public:
    Matrix<4, 4> localTransform;
    Shape *shape; // nullable
    std::vector<std::shared_ptr<SceneNode>> children;

    explicit SceneNode(Shape *s = nullptr)
        : localTransform(getIdentity4()), shape(s) {}

    // Add a child and return a raw pointer for convenience
    SceneNode *addChild(std::shared_ptr<SceneNode> child)
    {
        children.push_back(std::move(child));
        return children.back().get();
    }

    // Create and add a child in one call
    SceneNode *addChild(Shape *s = nullptr)
    {
        return addChild(std::make_shared<SceneNode>(s));
    }

    //  Draw
    // parentWorld: the accumulated world transform from above this node.
    // shaderID: the active program – used to set "modelMatrix" uniform.
    // wireframe: if true call drawWireframe(), else drawFilled().
    void draw(const Matrix<4, 4> &parentWorld,
              GLuint shaderID,
              bool wireframe) const
    {
        Matrix<4, 4> world = parentWorld * localTransform;

        // Upload model matrix
        float flat[16];
        flattenMatrix4(world, flat);
        GLint loc = glGetUniformLocation(shaderID, "modelMatrix");
        glUniformMatrix4fv(loc, 1, GL_FALSE, flat);

        // Draw own geometry
        if (shape)
        {
            if (wireframe)
                shape->drawWireframe();
            else
                shape->drawFilled();
        }

        // Recurse into children
        for (auto &child : children)
            child->draw(world, shaderID, wireframe);
    }

    // Overload for convenience 
    void draw(GLuint shaderID, bool wireframe) const
    {
        draw(getIdentity4(), shaderID, wireframe);
    }
};

#endif // SCENENODE_H
