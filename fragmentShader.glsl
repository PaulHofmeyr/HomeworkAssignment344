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

// ── Spotlight shadow maps ─────────────────────────────────────────────────────
// One 2D texture per shadow-casting spotlight, packed into an array.
// Index i in this array corresponds to spotlight i in spotLights[].
// MAX_SPOT_SHADOWS must match the value in Lighting.h.
#define MAX_SPOT_SHADOWS 16
uniform sampler2DArray spotShadowMaps;          // texture unit 2
uniform mat4           spotLightSpaceMatrix[MAX_SPOT_SHADOWS];
uniform int            spotShadowLayer[MAX_SPOTLIGHTS];
uniform int            numSpotShadows;          // how many layers are valid
uniform bool           useSpotShadows;

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

// shadowIndex: which layer of the spotShadowMaps array to sample
float calcSpotShadow(int shadowIndex, vec3 normal, vec3 lightDir)
{
    if (shadowIndex < 0 || shadowIndex >= numSpotShadows)
        return 1.0;

    vec4 fragPosLS = spotLightSpaceMatrix[shadowIndex] * vec4(fragPos, 1.0);

    // Perspective divide → NDC, then remap to [0,1]
    vec3 proj = fragPosLS.xyz / fragPosLS.w;
    proj = proj * 0.5 + 0.5;

    // Fragment is outside this spotlight's frustum — not shadowed
    if (proj.z > 1.0 || proj.x < 0.0 || proj.x > 1.0
                     || proj.y < 0.0 || proj.y > 1.0)
        return 1.0;

    // Sample the correct layer of the depth array
    float closest = texture(spotShadowMaps, vec3(proj.xy, float(shadowIndex))).r;
    float current = proj.z;

    // Angle-dependent bias prevents shadow acne on slanted surfaces
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

// spotIndex: position in spotLights[] — used to find the matching shadow layer
vec3 calcSpotLight(SpotLight light, int spotIndex, vec3 normal, vec3 viewDir, vec3 baseDiffuse)
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

    // Apply shadow only inside the cone and only if a shadow map exists
    float shadow = 1.0;
    int shadowIndex = spotShadowLayer[spotIndex];
    if (useSpotShadows && intensity > 0.0 && shadowIndex >= 0)
        shadow = calcSpotShadow(shadowIndex, normal, lightDir);

    return (amb + (dif + spe) * intensity * shadow) * atten;
}

// ─────────────────────────────────────────────────────────────────────────────
void main()
{
    vec3 norm     = normalize(fragNormal);
    vec3 viewDir  = normalize(viewPos - fragPos);
    vec3 baseDiff = useMaterialOverride ? material.diffuse : fragColour;

    vec3 result = vec3(0.0);

    result += calcDirLight(sun, norm, viewDir, baseDiff);

    for (int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; i++)
        result += calcPointLight(pointLights[i], norm, viewDir, baseDiff);

    for (int i = 0; i < numSpotLights && i < MAX_SPOTLIGHTS; i++)
        result += calcSpotLight(spotLights[i], i, norm, viewDir, baseDiff);

    colour = vec4(result, 1.0);
}
