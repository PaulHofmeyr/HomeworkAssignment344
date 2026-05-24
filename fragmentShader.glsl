#version 330 core

in vec3 fragPos;
in vec3 fragNormal;
in vec3 fragColour;

out vec4 colour;

// ── Material ──────────────────────────────────────────────────────────────────
struct Material {
    vec3  ambient;
    vec3  diffuse;
    vec3  specular;
    float shininess;
};
uniform Material material;
uniform bool      useMaterialOverride;

// ── Sun / directional light ───────────────────────────────────────────────────
struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    bool enabled;
};
uniform DirLight sun;

// ── Point lights ──────────────────────────────────────────────────────────────
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

// ── Spotlights ────────────────────────────────────────────────────────────────
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

// ── Camera ────────────────────────────────────────────────────────────────────
uniform vec3 viewPos;

// ── Sun shadow map ────────────────────────────────────────────────────────────
uniform sampler2D sunShadowMap;
uniform mat4      sunLightSpaceMatrix;
uniform bool      useSunShadow;

// ── Drone / spotlight shadow map ──────────────────────────────────────────────
// Single 2D depth map for the drone spotlight (index numSpotLights-1).
uniform sampler2D spotShadowMap;
uniform mat4      spotLightSpaceMatrix;
uniform bool      useSpotShadow;

// ─────────────────────────────────────────────────────────────────────────────
float calcSunShadow(vec4 fragPosLS, vec3 normal, vec3 lightDir)
{
    vec3 proj = fragPosLS.xyz / fragPosLS.w;
    proj = proj * 0.5 + 0.5;
    if (proj.z > 1.0) return 1.0;

    float closest = texture(sunShadowMap, proj.xy).r;
    float current = proj.z;
    float bias    = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);
    return (current - bias > closest) ? 0.0 : 1.0;
}

float calcSpotShadow(vec3 normal, vec3 lightDir)
{
    vec4 fragPosLS = spotLightSpaceMatrix * vec4(fragPos, 1.0);
    vec3 proj = fragPosLS.xyz / fragPosLS.w;
    proj = proj * 0.5 + 0.5;

    // Outside the spotlight frustum → not shadowed
    if (proj.z > 1.0 || proj.x < 0.0 || proj.x > 1.0
                     || proj.y < 0.0 || proj.y > 1.0)
        return 1.0;

    float closest = texture(spotShadowMap, proj.xy).r;
    float current = proj.z;
    // Angle-dependent bias (steeper surfaces need more bias)
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.001);
    return (current - bias > closest) ? 0.0 : 1.0;
}

// ─────────────────────────────────────────────────────────────────────────────
vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 baseDiffuse)
{
    if (!light.enabled) return vec3(0.0);

    vec3  lightDir = normalize(light.direction);
    float diff     = max(dot(normal, lightDir), 0.0);
    vec3  halfDir  = normalize(lightDir + viewDir);
    float spec     = pow(max(dot(normal, halfDir), 0.0), material.shininess);

    vec3 amb = light.ambient  * baseDiffuse;
    vec3 dif = light.diffuse  * diff * baseDiffuse;
    vec3 spe = light.specular * spec * material.specular;

    float shadow = 1.0;
    if (useSunShadow) {
        vec4 fragPosLS = sunLightSpaceMatrix * vec4(fragPos, 1.0);
        shadow = calcSunShadow(fragPosLS, normal, lightDir);
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

    float diff    = max(dot(normal, lightDir), 0.0);
    vec3  halfDir = normalize(lightDir + viewDir);
    float spec    = pow(max(dot(normal, halfDir), 0.0), material.shininess);

    vec3 amb = light.ambient  * baseDiffuse;
    vec3 dif = light.diffuse  * diff * baseDiffuse;
    vec3 spe = light.specular * spec * material.specular;
    return (amb + dif + spe) * atten;
}

// isDroneSpot: true only for the last spotlight (the drone), which has a shadow map.
vec3 calcSpotLight(SpotLight light, bool isDroneSpot, vec3 normal, vec3 viewDir, vec3 baseDiffuse)
{
    if (!light.enabled) return vec3(0.0);

    vec3  toLight  = light.position - fragPos;
    float dist     = length(toLight);
    vec3  lightDir = normalize(toLight);
    float atten    = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

    float theta     = dot(lightDir, normalize(-light.direction));
    float epsilon   = light.cutOffInner - light.cutOffOuter;
    float intensity = clamp((theta - light.cutOffOuter) / epsilon, 0.0, 1.0);

    float diff    = max(dot(normal, lightDir), 0.0);
    vec3  halfDir = normalize(lightDir + viewDir);
    float spec    = pow(max(dot(normal, halfDir), 0.0), material.shininess);

    vec3 amb = light.ambient  * baseDiffuse;
    vec3 dif = light.diffuse  * diff * baseDiffuse;
    vec3 spe = light.specular * spec * material.specular;

    float shadow = 1.0;
    if (isDroneSpot && useSpotShadow && intensity > 0.0)
        shadow = calcSpotShadow(normal, lightDir);

    return (amb + (dif + spe) * intensity * shadow) * atten;
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
    {
        bool isDrone = (i == numSpotLights - 1);
        result += calcSpotLight(spotLights[i], isDrone, norm, viewDir, baseDiffuse);
    }

    colour = vec4(result, 1.0);
}
