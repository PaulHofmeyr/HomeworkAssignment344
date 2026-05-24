#ifndef SHAPE_H
#define SHAPE_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>

// Vertex layout: [x y z | nx ny nz | r g b]  (9 floats)

class Shape
{
protected:
    GLuint filledVAO, filledVBO;
    int    filledVertexCount;

    GLuint wireVAO, wireVBO;
    int    wireVertexCount;

    float r, g, b;
    bool  isClone = false;   // clones share GPU buffers — do not delete them

    GLuint uploadToGPU(const std::vector<float> &data, GLuint &vboOut);
    void   buildBuffers(const std::vector<float> &filledData,
                        const std::vector<float> &wireData);

    // Helpers used by derived classes
    void pushVertex  (std::vector<float> &vec,
                      float x, float y, float z,
                      float nx, float ny, float nz) const;

    // Triangle with auto-computed face normal
    void pushTriangle(std::vector<float> &vec,
                      float x0, float y0, float z0,
                      float x1, float y1, float z1,
                      float x2, float y2, float z2) const;

    // Line (normal = 0,1,0 - irrelevant for wireframe)
    void pushLine    (std::vector<float> &vec,
                      float x0, float y0, float z0,
                      float x1, float y1, float z1) const;

public:
    Shape(float r, float g, float b);
    virtual ~Shape();
    virtual void build() = 0;
    void drawFilled()    const;
    void drawWireframe() const;

    // ── Prototype pattern ──────────────────────────────────────────────────
    // Copy GPU buffer handles from a built prototype so geometry does not need
    // to be recomputed.  The clone shares the VAO/VBO (read-only rendering),
    // so do NOT delete the prototype before all clones are done with it.
    void cloneBuffers(const Shape &proto)
    {
        filledVAO         = proto.filledVAO;
        filledVBO         = proto.filledVBO;
        filledVertexCount = proto.filledVertexCount;
        wireVAO           = proto.wireVAO;
        wireVBO           = proto.wireVBO;
        wireVertexCount   = proto.wireVertexCount;
        isClone           = true;
    }
};

#endif // SHAPE_H
