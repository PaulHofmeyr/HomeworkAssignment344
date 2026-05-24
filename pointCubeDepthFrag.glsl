#version 330 core
// pointCubeDepthFrag.glsl
// Writes the linear distance from the light so the cubemap stores
// true distance rather than the clip-space depth non-linearity.

in  vec3  fragPos;

uniform vec3  lightPos;
uniform float farPlane;

void main()
{
    float dist = length(fragPos - lightPos);
    gl_FragDepth = dist / farPlane;   // normalise to [0,1]
}
