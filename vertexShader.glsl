#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec3 vertexColour;

out vec3 fragPos;
out vec3 fragNormal;
out vec3 fragColour;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
    vec4 worldPos = modelMatrix * vec4(vertexPosition, 1.0);
    fragPos       = worldPos.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    fragNormal = normalize(normalMatrix * vertexNormal);

    fragColour = vertexColour;

    gl_Position = projectionMatrix * viewMatrix * worldPos;
}
