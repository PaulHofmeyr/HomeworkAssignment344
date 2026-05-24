#version 330 core
// pointCubeDepthGeom.glsl
// Emits each triangle once per cube face so one draw call fills all 6 layers.

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

// 6 light-space matrices, one per cube face (+X -X +Y -Y +Z -Z)
uniform mat4 shadowMatrices[6];

in  vec3 worldPos[];   // from vertex shader
out vec3 fragPos;      // world-space position for distance test in frag shader

void main()
{
    for (int face = 0; face < 6; face++)
    {
        gl_Layer = face;
        for (int v = 0; v < 3; v++)
        {
            fragPos     = worldPos[v];
            gl_Position = shadowMatrices[face] * vec4(worldPos[v], 1.0);
            EmitVertex();
        }
        EndPrimitive();
    }
}
