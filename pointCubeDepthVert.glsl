#version 330 core
// pointCubeDepthVert.glsl
// Transforms each vertex into world space; the geometry shader
// then projects it onto all 6 cubemap faces at once.
layout(location = 0) in vec3 vertexPosition;

uniform mat4 modelMatrix;

out vec3 worldPos;

void main()
{
    worldPos = vec3(modelMatrix * vec4(vertexPosition, 1.0));
}
