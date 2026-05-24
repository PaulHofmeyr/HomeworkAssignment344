#version 330 core

in vec3 fragPos;
in vec3 fragNormal;
in vec3 fragColour;

out vec4 colour;

// ── Material ─────────────────────────────────────────────────────────────────
struct Material {
    vec3  ambient;
    vec3  diffuse;
    vec3  specular;
    float shininess;
};
uniform Material material;
uniform bool      useMaterialOverride;

// ── Sun / directional light ──────────────────────────────────────────────────
struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    bool enabled;
};
uniform DirLight sun;

// ── Point lights ─────────────────────────────────────────────────────────────
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

// ── Spotlights ───────────────────────────────────────────────────────────────
#define MAX_SPOTLIGHTS 32
struct SpotLight {
    vec3  position;
    vec3  direction;
    float cutOffInner;
    float cutOffOuter;
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

// ── Camera ───────────────────────────────────────────────────────────────────
uniform vec3 viewPos;

// ── Sun shadow map ───────────────────────────────────────────────────────────
uniform sampler2D   sunShadowMap;
uniform mat4        sunLightSpaceMatrix;
uniform bool        useSunShadow;

// Returns 1.0 = fully lit, 0.0 = fully shadowed
float calcSunShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    // Perspective divide → NDC [-1,1]
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Map to [0,1] for texture lookup
    projCoords = projCoords * 0.5 + 0.5;

    // Outside the light frustum → not in shadow
    if (projCoords.z > 1.0)
        return 1.0;

    float closestDepth = texture(sunShadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    // Bias to prevent shadow acne — scales with surface angle to light
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);

    // Simple single-sample comparison
    return (currentDepth - bias > closestDepth) ? 0.0 : 1.0;
}

// ─────────────────────────────────────────────────────────────────────────────
vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 baseDiffuse)
{
    if (!light.enabled) return vec3(0.0);

    vec3  lightDir = normalize(light.direction);
    float diff     = max(dot(normal, lightDir), 0.0);

    vec3  halfDir = normalize(lightDir + viewDir);
    float spec    = pow(max(dot(normal, halfDir), 0.0), material.shininess);

    vec3 amb = light.ambient  * baseDiffuse;
    vec3 dif = light.diffuse  * diff * baseDiffuse;
    vec3 spe = light.specular * spec * material.specular;

    // Shadow: only attenuate diffuse + specular, keep ambient
    float shadow = 1.0;
    if (useSunShadow) {
        vec4 fragPosLightSpace = sunLightSpaceMatrix * vec4(fragPos, 1.0);
        shadow = calcSunShadow(fragPosLightSpace, normal, lightDir);
    }

    return amb + (dif + spe) * shadow;
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 baseDiffuse)
{
    if (!light.enabled) return vec3(0.0);

    vec3  toLight  = light.position - fragPos;
    float dist     = length(toLight);
    vec3  lightDir = normalize(toLight);
    float atten    = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

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

    float theta     = dot(lightDir, normalize(-light.direction));
    float epsilon   = light.cutOffInner - light.cutOffOuter;
    float intensity = clamp((theta - light.cutOffOuter) / epsilon, 0.0, 1.0);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3  halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0), material.shininess);

    vec3 amb = light.ambient  * baseDiffuse;
    vec3 dif = light.diffuse  * diff * baseDiffuse;
    vec3 spe = light.specular * spec * material.specular;
    return (amb + (dif + spe) * intensity) * atten;
}

// ─────────────────────────────────────────────────────────────────────────────
void main()
{
    vec3 norm    = normalize(fragNormal);
    vec3 viewDir = normalize(viewPos - fragPos);

    vec3 baseDiffuse = useMaterialOverride ? material.diffuse : fragColour;

    vec3 result = vec3(0.0);

    result += calcDirLight(sun, norm, viewDir, baseDiffuse);

    for (int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; i++)
        result += calcPointLight(pointLights[i], norm, viewDir, baseDiffuse);

    for (int i = 0; i < numSpotLights && i < MAX_SPOTLIGHTS; i++)
        result += calcSpotLight(spotLights[i], norm, viewDir, baseDiffuse);

    colour = vec4(result, 1.0);
}
