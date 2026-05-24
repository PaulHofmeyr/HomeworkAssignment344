#version 330 core

in  vec3 texCoords;
out vec4 colour;

uniform samplerCube skybox;

void main()
{
    colour = texture(skybox, texCoords);
}
