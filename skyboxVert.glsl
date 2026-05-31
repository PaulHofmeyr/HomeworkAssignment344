#version 330 core

layout(location = 0) in vec3 vertexPosition;

out vec3 texCoords;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
    texCoords = vertexPosition;

    // Remove translation from the view matrix so the skybox is always
    // centred on the camera and never appears to move.
    mat4 rotView = mat4(mat3(viewMatrix));

    vec4 pos = projectionMatrix * rotView * vec4(vertexPosition, 1.0);

    // Set z = w so the skybox is always at the far plane (depth = 1.0)
    gl_Position = pos.xyww;
}
