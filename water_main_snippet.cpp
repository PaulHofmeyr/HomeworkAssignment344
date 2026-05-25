// ── ADD to main.cpp ──────────────────────────────────────────

// 1. After loading other shaders, load the water shader:
GLuint waterShaderID = LoadShaders("waterVert.glsl", "waterFrag.glsl");

// 2. Inside the render loop, draw water separately:

// Enable blending for water transparency
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

glUseProgram(waterShaderID);

// Upload shared matrices
setMat4(waterShaderID, "modelMatrix",      getIdentity4());
setMat4(waterShaderID, "viewMatrix",       view);
setMat4(waterShaderID, "projectionMatrix", proj);

// Upload time for animation
glUniform1f(glGetUniformLocation(waterShaderID, "time"),
            (float)glfwGetTime());

// Upload sun direction
glUniform3f(glGetUniformLocation(waterShaderID, "sunDir"),
            -0.3f, 0.8f, -0.5f);  // matches your sun light direction

// Upload camera pos
glUniform3f(glGetUniformLocation(waterShaderID, "viewPos"),
            dx, dy, dz);

// Draw water
buildWaterNode — already a SceneNode, just draw it with the water shader:
g_waterNode->draw(waterShaderID, app.wireframe);

glDisable(GL_BLEND);

// 3. At cleanup:
glDeleteProgram(waterShaderID);

// ── In buildSceneRoot(), store water node separately: ────────
// Add to SceneRoot.h:
extern std::shared_ptr<SceneNode> g_waterNode;

// In SceneRoot.cpp buildSceneRoot():
g_waterNode = buildWaterNode(*g_layout);
// Don't add to g_root — draw it separately in main.cpp

