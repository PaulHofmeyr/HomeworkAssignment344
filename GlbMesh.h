#ifndef GLBMESH_H
#define GLBMESH_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <string>
#include <cmath>
#include <iostream>
#include "Transformations.h"

// ---------------------------------------------------------------------------
// GlbMesh
//
// Loads a .glb (or any Assimp-supported format) and renders it using the
// same 9-float vertex layout as the procedural Shape classes:
//   [x y z | nx ny nz | r g b]
//
// Each Assimp mesh becomes one VAO.  The colour comes from the mesh's
// diffuse material colour; if none is embedded a neutral grey is used.
//
// Usage:
//   GlbMesh model("assets/crate.glb");
//   model.setTransform(makeTranslation3D(2, 0, -1));
//   // in draw loop:
//   model.draw(shaderID, wireframe);
// ---------------------------------------------------------------------------

struct GpuMesh
{
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;           // index buffer (may be 0 if unused)
    GLsizei indexCount  = 0;
    GLsizei vertexCount = 0;  // used when no index buffer
    bool    useIndices  = false;
};

class GlbMesh
{
public:
    // ctor – loads immediately; check isLoaded() before use
    explicit GlbMesh(const std::string& path,
                     float defaultR = 0.75f,
                     float defaultG = 0.75f,
                     float defaultB = 0.75f);

    ~GlbMesh();

    // Prevent copies (GPU resources)
    GlbMesh(const GlbMesh&)            = delete;
    GlbMesh& operator=(const GlbMesh&) = delete;

    bool isLoaded() const { return loaded_; }

    // World transform applied before drawing (set this each frame or once)
    void setTransform(const Matrix<4,4>& m) { transform_ = m; }
    const Matrix<4,4>& getTransform() const { return transform_; }

    // Request wood texture overlay for this mesh
    void setApplyWood(bool v) { applyWood_ = v; }

    // Draw helpers
    // shaderID must already be bound (glUseProgram called by caller).
    // These upload "modelMatrix" and draw all sub-meshes.
    void draw        (GLuint shaderID, bool wireframe = false) const;
    void drawFilled  (GLuint shaderID) const;
    void drawWireframe(GLuint shaderID) const;

    // Draw using a *separate* depth-only shader (shadow pass).
    // Uploads only "modelMatrix"; the depth shader only needs positions
    // which sit at attrib 0 with stride 9 floats – same layout.
    void drawDepth(GLuint depthShaderID) const;

private:
    bool          loaded_     = false;
    bool          applyWood_ = false;
    Matrix<4,4>   transform_;
    std::vector<GpuMesh> meshes_;

    float defR_, defG_, defB_;

    void processNode(aiNode* node, const aiScene* scene);
    GpuMesh processMesh(aiMesh* mesh, const aiScene* scene);

    void uploadModelMatrix(GLuint shaderID) const;
    void drawMeshes(bool wireframe) const;
};

#endif // GLBMESH_H
