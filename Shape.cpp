#include "Shape.h"
#include <cmath>

Shape::Shape(float r, float g, float b)
    : filledVAO(0), filledVBO(0), filledVertexCount(0),
      wireVAO(0),   wireVBO(0),   wireVertexCount(0),
      r(r), g(g), b(b)
{}

Shape::~Shape()
{
    if (filledVAO) glDeleteVertexArrays(1, &filledVAO);
    if (filledVBO) glDeleteBuffers(1, &filledVBO);
    if (wireVAO)   glDeleteVertexArrays(1, &wireVAO);
    if (wireVBO)   glDeleteBuffers(1, &wireVBO);
}

// Layout: pos(3) | normal(3) | colour(3)  = 9 floats per vertex
GLuint Shape::uploadToGPU(const std::vector<float> &data, GLuint &vboOut)
{
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vboOut);
    glBindBuffer(GL_ARRAY_BUFFER, vboOut);
    glBufferData(GL_ARRAY_BUFFER,
                 data.size() * sizeof(float),
                 data.data(),
                 GL_STATIC_DRAW);

    const GLsizei stride = 9 * sizeof(float);
    // attrib 0: position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    // attrib 1: normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // attrib 2: colour
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    return vao;
}

void Shape::buildBuffers(const std::vector<float> &filledData,
                         const std::vector<float> &wireData)
{
    filledVertexCount = (int)filledData.size() / 9;
    filledVAO = uploadToGPU(filledData, filledVBO);

    wireVertexCount = (int)wireData.size() / 9;
    if(wireVertexCount > 0)
        wireVAO = uploadToGPU(wireData, wireVBO);
}

void Shape::pushVertex(std::vector<float> &vec,
                        float x, float y, float z,
                        float nx, float ny, float nz) const
{
    vec.push_back(x);  vec.push_back(y);  vec.push_back(z);
    vec.push_back(nx); vec.push_back(ny); vec.push_back(nz);
    vec.push_back(r);  vec.push_back(g);  vec.push_back(b);
}

void Shape::pushTriangle(std::vector<float> &vec,
                          float x0, float y0, float z0,
                          float x1, float y1, float z1,
                          float x2, float y2, float z2) const
{
    // Compute face normal
    float ax = x1 - x0, ay = y1 - y0, az = z1 - z0;
    float bx = x2 - x0, by = y2 - y0, bz = z2 - z0;
    float nx = ay * bz - az * by;
    float ny = az * bx - ax * bz;
    float nz = ax * by - ay * bx;
    float len = std::sqrt(nx*nx + ny*ny + nz*nz);
    if (len > 1e-6f) { nx /= len; ny /= len; nz /= len; }

    pushVertex(vec, x0, y0, z0, nx, ny, nz);
    pushVertex(vec, x1, y1, z1, nx, ny, nz);
    pushVertex(vec, x2, y2, z2, nx, ny, nz);
}

void Shape::pushLine(std::vector<float> &vec,
                      float x0, float y0, float z0,
                      float x1, float y1, float z1) const
{
    pushVertex(vec, x0, y0, z0, 0.f, 1.f, 0.f);
    pushVertex(vec, x1, y1, z1, 0.f, 1.f, 0.f);
}

void Shape::drawFilled() const
{
    glBindVertexArray(filledVAO);
    glDrawArrays(GL_TRIANGLES, 0, filledVertexCount);
    glBindVertexArray(0);
}

void Shape::drawWireframe() const
{
    if(!wireVAO || wireVertexCount <= 0)
        return;
    glBindVertexArray(wireVAO);
    glDrawArrays(GL_LINES, 0, wireVertexCount);
    glBindVertexArray(0);
}
