#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec3 vertexColour;
layout(location = 3) in vec2 vertexUV;

out vec3 fragPos;
out vec3 fragNormal;
out vec3 fragColour;
out vec2 fragUV;

uniform mat4  modelMatrix;
uniform mat4  viewMatrix;
uniform mat4  projectionMatrix;
uniform float time;

void main()
{
    // ── Vertex wave displacement ──────────────────────────────
    //  Gently bob the water surface vertices up and down.
    //  Only displaces Y (height) so the outline stays correct.
    vec3 pos   = vertexPosition;
    float wave = sin(pos.x * 3.0f + time * 1.2f)
               * cos(pos.z * 2.5f + time * 0.9f) * 0.04f;
    pos.y     += wave;

    vec4 worldPos  = modelMatrix * vec4(pos, 1.0);
    fragPos        = worldPos.xyz;
    fragNormal     = normalize(mat3(transpose(inverse(mat3(modelMatrix))))
                               * vertexNormal);
    fragColour     = vertexColour;
    fragUV         = vertexUV;
    gl_Position    = projectionMatrix * viewMatrix * worldPos;
}
