#version 330 core

in vec3 fragPos;
in vec3 fragNormal;
in vec3 fragColour;

out vec4 colour;

// ── Material ────────────────────────────────────────────────────────────────
struct Material {
    vec3  ambient;
    vec3  diffuse;
    vec3  specular;
    float shininess;
};
uniform Material material;
uniform bool      useMaterialOverride; // false → use vertex colour as diffuse

// ── Sun / directional light ─────────────────────────────────────────────────
struct DirLight {
    vec3 direction;   // world-space, pointing TOWARD the light
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    bool enabled;
};
uniform DirLight sun;

// ── Point lights (bollards / under-gazebo) ──────────────────────────────────
#define MAX_POINT_LIGHTS 8
struct PointLight {
    vec3  position;
    vec3  ambient;
    vec3  diffuse;
    vec3  specular;
    float constant;
    float linear;
    float quadratic;
    bool  enabled;
};
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int        numPointLights;

// ── Spotlights (floodlight clusters + drone light) ──────────────────────────
#define MAX_SPOTLIGHTS 32
struct SpotLight {
    vec3  position;
    vec3  direction;
    float cutOffInner;   // cos of inner half-angle
    float cutOffOuter;   // cos of outer half-angle
    vec3  ambient;
    vec3  diffuse;
    vec3  specular;
    float constant;
    float linear;
    float quadratic;
    bool  enabled;
};
uniform SpotLight spotLights[MAX_SPOTLIGHTS];
uniform int       numSpotLights;

// ── Camera ──────────────────────────────────────────────────────────────────
uniform vec3 viewPos;

// ────────────────────────────────────────────────────────────────────────────
vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 baseDiffuse)
{
    if (!light.enabled) return vec3(0.0);

    vec3 lightDir = normalize(light.direction);
    float diff    = max(dot(normal, lightDir), 0.0);

    // Blinn-Phong half-vector
    vec3  halfDir = normalize(lightDir + viewDir);
    float spec    = pow(max(dot(normal, halfDir), 0.0), material.shininess);

    vec3 amb  = light.ambient  * baseDiffuse;
    vec3 dif  = light.diffuse  * diff * baseDiffuse;
    vec3 spe  = light.specular * spec * material.specular;
    return amb + dif + spe;
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 baseDiffuse)
{
    if (!light.enabled) return vec3(0.0);

    vec3  toLight = light.position - fragPos;
    float dist    = length(toLight);
    vec3  lightDir = normalize(toLight);
    float atten   = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3  halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0), material.shininess);

    vec3 amb = light.ambient  * baseDiffuse;
    vec3 dif = light.diffuse  * diff * baseDiffuse;
    vec3 spe = light.specular * spec * material.specular;
    return (amb + dif + spe) * atten;
}

vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 viewDir, vec3 baseDiffuse)
{
    if (!light.enabled) return vec3(0.0);

    vec3  toLight  = light.position - fragPos;
    float dist     = length(toLight);
    vec3  lightDir = normalize(toLight);
    float atten    = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

    // Spot cone
    float theta   = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOffInner - light.cutOffOuter;
    float intensity = clamp((theta - light.cutOffOuter) / epsilon, 0.0, 1.0);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3  halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0), material.shininess);

    vec3 amb = light.ambient  * baseDiffuse;
    vec3 dif = light.diffuse  * diff * baseDiffuse;
    vec3 spe = light.specular * spec * material.specular;
    return (amb + (dif + spe) * intensity) * atten;
}

// ────────────────────────────────────────────────────────────────────────────
void main()
{
    vec3 norm    = normalize(fragNormal);
    vec3 viewDir = normalize(viewPos - fragPos);

    // Base diffuse colour: either from material or vertex colour
    vec3 baseDiffuse = useMaterialOverride ? material.diffuse : fragColour;

    vec3 result = vec3(0.0);

    result += calcDirLight(sun, norm, viewDir, baseDiffuse);

    for (int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; i++)
        result += calcPointLight(pointLights[i], norm, viewDir, baseDiffuse);

    for (int i = 0; i < numSpotLights && i < MAX_SPOTLIGHTS; i++)
        result += calcSpotLight(spotLights[i], norm, viewDir, baseDiffuse);

    colour = vec4(result, 1.0);
}
