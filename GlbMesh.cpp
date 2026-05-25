#include "GlbMesh.h"

// ---------------------------------------------------------------------------
GlbMesh::GlbMesh(const std::string& path,
                 float defaultR, float defaultG, float defaultB)
    : transform_(getIdentity4()),
      defR_(defaultR), defG_(defaultG), defB_(defaultB)
{
    Assimp::Importer importer;

    // Triangulate + generate missing normals + flip UVs for GL convention
    const unsigned int flags =
        aiProcess_Triangulate          |
        aiProcess_GenSmoothNormals     |
        aiProcess_FlipUVs              |
        aiProcess_JoinIdenticalVertices|
        aiProcess_CalcTangentSpace;

    const aiScene* scene = importer.ReadFile(path, flags);

    if (!scene || !scene->mRootNode ||
        (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE))
    {
        std::cerr << "[GlbMesh] Failed to load \"" << path << "\": "
                  << importer.GetErrorString() << "\n";
        loaded_ = false;
        return;
    }

    processNode(scene->mRootNode, scene);
    loaded_ = true;
    std::cout << "[GlbMesh] Loaded \"" << path << "\" ("
              << meshes_.size() << " mesh(es))\n";
}

// ---------------------------------------------------------------------------
GlbMesh::~GlbMesh()
{
    for (auto& m : meshes_)
    {
        if (m.vao) glDeleteVertexArrays(1, &m.vao);
        if (m.vbo) glDeleteBuffers(1, &m.vbo);
        if (m.ebo) glDeleteBuffers(1, &m.ebo);
    }
}

// ---------------------------------------------------------------------------
void GlbMesh::processNode(aiNode* node, const aiScene* scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes_.push_back(processMesh(mesh, scene));
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++)
        processNode(node->mChildren[i], scene);
}

// ---------------------------------------------------------------------------
GpuMesh GlbMesh::processMesh(aiMesh* mesh, const aiScene* scene)
{
    // ── Fetch material colour ────────────────────────────────────────────────
    float cr = defR_, cg = defG_, cb = defB_;
    if (mesh->mMaterialIndex < scene->mNumMaterials)
    {
        aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
        aiColor4D diffuse;
        if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
        {
            cr = diffuse.r;
            cg = diffuse.g;
            cb = diffuse.b;
        }
    }

    // ── Build interleaved vertex buffer [pos(3) | normal(3) | colour(3)] ────
    std::vector<float> verts;
    verts.reserve(mesh->mNumVertices * 9);

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        // Position
        verts.push_back(mesh->mVertices[i].x);
        verts.push_back(mesh->mVertices[i].y);
        verts.push_back(mesh->mVertices[i].z);

        // Normal
        if (mesh->HasNormals())
        {
            verts.push_back(mesh->mNormals[i].x);
            verts.push_back(mesh->mNormals[i].y);
            verts.push_back(mesh->mNormals[i].z);
        }
        else
        {
            verts.push_back(0.f); verts.push_back(1.f); verts.push_back(0.f);
        }

        // Colour
        verts.push_back(cr);
        verts.push_back(cg);
        verts.push_back(cb);
    }

    // ── Build index buffer ────────────────────────────────────────────────────
    std::vector<unsigned int> indices;
    indices.reserve(mesh->mNumFaces * 3);
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace& face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // ── Upload to GPU ─────────────────────────────────────────────────────────
    GpuMesh gm;
    const GLsizei stride = 9 * sizeof(float);

    glGenVertexArrays(1, &gm.vao);
    glBindVertexArray(gm.vao);

    glGenBuffers(1, &gm.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, gm.vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)(verts.size() * sizeof(float)),
                 verts.data(), GL_STATIC_DRAW);

    // attrib 0 – position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    // attrib 1 – normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                          (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // attrib 2 – colour
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride,
                          (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    if (!indices.empty())
    {
        glGenBuffers(1, &gm.ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gm.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     (GLsizeiptr)(indices.size() * sizeof(unsigned int)),
                     indices.data(), GL_STATIC_DRAW);
        gm.indexCount = (GLsizei)indices.size();
        gm.useIndices = true;
    }
    else
    {
        gm.vertexCount = (GLsizei)mesh->mNumVertices;
        gm.useIndices  = false;
    }

    glBindVertexArray(0);
    return gm;
}

// ---------------------------------------------------------------------------
void GlbMesh::uploadModelMatrix(GLuint shaderID) const
{
    float flat[16];
    flattenMatrix4(transform_, flat);
    GLint loc = glGetUniformLocation(shaderID, "modelMatrix");
    glUniformMatrix4fv(loc, 1, GL_FALSE, flat);
}

// ---------------------------------------------------------------------------
void GlbMesh::drawMeshes(bool wireframe) const
{
    GLenum mode = wireframe ? GL_LINE_LOOP : GL_TRIANGLES;

    for (const auto& m : meshes_)
    {
        glBindVertexArray(m.vao);
        if (m.useIndices)
            glDrawElements(mode, m.indexCount, GL_UNSIGNED_INT, 0);
        else
            glDrawArrays(mode, 0, m.vertexCount);
        glBindVertexArray(0);
    }
}

// ---------------------------------------------------------------------------
void GlbMesh::draw(GLuint shaderID, bool wireframe) const
{
    if (!loaded_) return;
    uploadModelMatrix(shaderID);
    GLint isGlbLoc  = glGetUniformLocation(shaderID, "isGlbMesh");
    GLint woodLoc   = glGetUniformLocation(shaderID, "applyWoodToGlb");
    if (isGlbLoc >= 0) glUniform1i(isGlbLoc, 1);
    if (woodLoc  >= 0) glUniform1i(woodLoc,  applyWood_ ? 1 : 0);
    drawMeshes(wireframe);
    if (isGlbLoc >= 0) glUniform1i(isGlbLoc, 0);
    if (woodLoc  >= 0) glUniform1i(woodLoc,  0);
}

void GlbMesh::drawFilled(GLuint shaderID) const   { draw(shaderID, false); }
void GlbMesh::drawWireframe(GLuint shaderID) const { draw(shaderID, true);  }

// ---------------------------------------------------------------------------
void GlbMesh::drawDepth(GLuint depthShaderID) const
{
    if (!loaded_) return;
    uploadModelMatrix(depthShaderID);
    // Depth pass only needs positions — the VAO layout is compatible
    for (const auto& m : meshes_)
    {
        glBindVertexArray(m.vao);
        if (m.useIndices)
            glDrawElements(GL_TRIANGLES, m.indexCount, GL_UNSIGNED_INT, 0);
        else
            glDrawArrays(GL_TRIANGLES, 0, m.vertexCount);
        glBindVertexArray(0);
    }
}
