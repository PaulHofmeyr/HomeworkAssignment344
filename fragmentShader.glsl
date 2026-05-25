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

// ── Point-light cube shadow maps ──────────────────────────────────────────────
// samplerCube array requires GLSL 4.00; on 3.30 we declare individual samplers.
// Texture units 3..6 (one per possible shadow-casting point light).
#define MAX_POINT_CUBE_SHADOWS 4
#define MAX_POINT_LIGHTS 8
uniform samplerCube pointShadowCubeMap[MAX_POINT_CUBE_SHADOWS];  // tex units 3-6
uniform int         pointCubeShadowSlot[MAX_POINT_LIGHTS];
uniform int         numPointCubeShadows;
uniform float       pointShadowFarPlane;
uniform bool        usePointCubeShadows;

// ── Grass texture ─────────────────────────────────────────────────────────────
// Texture unit 7 — tiled over green surfaces using world-space XZ coordinates.
uniform sampler2D grassTex;      // tex unit 7
uniform bool      useGrassTex;

// ── Limestone texture (rocks / boulders) ──────────────────────────────────────
// Texture unit 8 — warm limestone mapped onto rocky/brownish surfaces.
uniform sampler2D limestoneTex;  // tex unit 8
uniform bool      useLimestoneTex;

// ── Concrete texture (paths / road) ───────────────────────────────────────────
// Texture unit 9 — grey concrete mapped onto the course walkway/road.
uniform sampler2D concreteTex;   // tex unit 9
uniform bool      useConcreteTex;

// ── Water texture ─────────────────────────────────────────────────────────────
// Texture unit 10 — animated scrolling texture on water surfaces.
uniform sampler2D waterTex;      // tex unit 10
uniform bool      useWaterTex;
uniform float     waterTime;     // glfwGetTime() — for UV animation

// ── Shrub texture ─────────────────────────────────────────────────────────────
// Texture unit 11 — leafy texture on shrubs, ornamental grass and tree canopy.
uniform sampler2D shrubTex;      // tex unit 11
uniform bool      useShrubTex;

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

// Cube shadow for a point light.
// cubeSlot: index into pointShadowCubeMap[] sampler array.
// lightPos: world-space position of the point light.
float calcPointCubeShadow(int cubeSlot, vec3 lightPos)
{
    if (!usePointCubeShadows || cubeSlot < 0 || cubeSlot >= numPointCubeShadows)
        return 1.0;

    // Direction from light to fragment — used to choose the correct cube face
    vec3  fragToLight = fragPos - lightPos;
    float currentDist = length(fragToLight) / pointShadowFarPlane;  // normalised

    // Sample closest stored depth for this direction
    // (GLSL requires a compile-time index into a sampler array; we use if/else)
    float closestDist;
    if      (cubeSlot == 0) closestDist = texture(pointShadowCubeMap[0], fragToLight).r;
    else if (cubeSlot == 1) closestDist = texture(pointShadowCubeMap[1], fragToLight).r;
    else if (cubeSlot == 2) closestDist = texture(pointShadowCubeMap[2], fragToLight).r;
    else                    closestDist = texture(pointShadowCubeMap[3], fragToLight).r;

    float bias = 0.05;
    return (currentDist - bias > closestDist) ? 0.0 : 1.0;
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

vec3 calcPointLight(PointLight light, int pointIndex, vec3 normal, vec3 viewDir, vec3 baseDiffuse)
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

    float shadow = calcPointCubeShadow(pointCubeShadowSlot[pointIndex], light.position);
    return (amb + (dif + spe) * shadow) * atten;
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

    // ── Grass texture on green surfaces ───────────────────────────────────────
    // Only bright putting greens / fairways (g > 0.62) get the grass texture.
    // Turf walls, shrubs, leaves use the shrub texture instead (see below).
    if (useGrassTex && !useMaterialOverride) {
        if (fragColour.g > 0.62 && fragColour.g > fragColour.r + 0.08) {
            vec2 grassUV     = fragPos.xz * 0.4;
            vec4 grassSample = texture(grassTex, grassUV);
            vec3 tinted      = grassSample.rgb * (fragColour * 2.0);
            baseDiff = mix(baseDiff, tinted, 0.72);
        }
    }

    // ── Limestone / rock texture on boulders and rock beds ────────────────────
    //   C_GRANITE_D = (0.38, 0.34, 0.30)  C_GRANITE_L = (0.55, 0.52, 0.48)
    //   C_SANDSTONE = (0.72, 0.58, 0.38)  COL_ROCK    = (0.40, 0.36, 0.30)
    //   Detection: warm brownish tone — red is highest, not too saturated
    if (useLimestoneTex) {
        float cr = fragColour.r, cg = fragColour.g, cb = fragColour.b;
        bool isRock = (cr > cb + 0.04)           // red > blue (warm tone)
                   && (cr > 0.25) && (cr < 0.80) // mid brightness
                   && (cg < cr + 0.02)            // green not dominant
                   && (cb > 0.10);               // not pure black
        if (isRock) {
            vec2 rockUV   = fragPos.xz * 0.4;
            vec3 lsSample = texture(limestoneTex, rockUV).rgb;
            // Blend texture directly — no dark tint multiplier
            baseDiff = mix(baseDiff, lsSample, 0.75);
        }
    }

    // ── Concrete texture on paths / road ──────────────────────────────────────
    // COL_ROAD = (0.55, 0.55, 0.55) — neutral grey, all channels nearly equal.
    if (useConcreteTex && !useMaterialOverride) {
        float cr = fragColour.r, cg = fragColour.g, cb = fragColour.b;
        float maxDiff = max(abs(cr - cg), max(abs(cr - cb), abs(cg - cb)));
        bool isRoad = (maxDiff < 0.06)                  // very neutral grey
                   && (cr > 0.45) && (cr < 0.72);       // mid-grey brightness band
        if (isRoad) {
            vec2 concreteUV = fragPos.xz * 0.35;        // ~2.8-unit tile
            vec4 conSample  = texture(concreteTex, concreteUV);
            baseDiff = mix(baseDiff, conSample.rgb, 0.70);
        }
    }

    // ── Water texture on water surfaces ───────────────────────────────────────
    // COL_WATER = (0.15, 0.55, 0.85) — strongly blue-dominant.
    // Two scrolling layers at different speeds/angles create animated rippling.
    if (useWaterTex && !useMaterialOverride) {
        float cr = fragColour.r, cg = fragColour.g, cb = fragColour.b;
        bool isWater = (cb > cg + 0.15) && (cb > cr + 0.35) && (cb > 0.55);
        if (isWater) {
            // Layer 1: slow diagonal drift
            vec2 uv1 = fragPos.xz * 0.18 + vec2(waterTime * 0.018,  waterTime * 0.012);
            // Layer 2: faster counter-drift for depth
            vec2 uv2 = fragPos.xz * 0.22 + vec2(-waterTime * 0.010, waterTime * 0.022);
            vec3 tex1 = texture(waterTex, uv1).rgb;
            vec3 tex2 = texture(waterTex, uv2).rgb;
            vec3 texBlend = (tex1 + tex2) * 0.5;
            // Blend: keep water's blue tint dominant, texture adds surface detail
            baseDiff = mix(baseDiff, texBlend, 0.55);
        }
    }

    // ── Shrub texture on turf walls, shrubs, ornamental grass and tree canopy ──
    //   TurfWall   = (0.12, 0.50, 0.14)  — the green walls
    //   C_SHRUB    = (0.22, 0.52, 0.18)  — shrub objects
    //   C_GRASS    = (0.38, 0.62, 0.22)  — ornamental grass objects
    //   C_LEAF     = (0.18, 0.52, 0.18)  — tree canopy
    //   Detection: mid-range green (0.45–0.63), clearly green-dominant, not too red
    if (useShrubTex) {
        float cr = fragColour.r, cg = fragColour.g, cb = fragColour.b;
        bool isShrub = (cg > cr + 0.20)   // strongly green-dominant
                    && (cg > cb + 0.20)    // more green than blue
                    && (cg < 0.63)         // not as bright as fairway greens
                    && (cr < 0.45);        // not warm/yellow tones
        if (isShrub) {
            // Triplanar UV: blend XZ (top), XY (front/back), ZY (left/right)
            // based on the surface normal so both flat tops and vertical walls
            // tile the texture correctly without stretching.
            float scale = 0.6;
            vec3 blendW = abs(norm);
            blendW = pow(blendW, vec3(4.0));          // sharpen blend seams
            blendW /= (blendW.x + blendW.y + blendW.z + 0.001);

            vec3 sXZ = texture(shrubTex, fragPos.xz * scale).rgb; // top face
            vec3 sXY = texture(shrubTex, fragPos.xy * scale).rgb; // front/back
            vec3 sZY = texture(shrubTex, fragPos.zy * scale).rgb; // left/right

            vec3 shrubSamp = sXZ * blendW.y + sXY * blendW.z + sZY * blendW.x;
            baseDiff = mix(baseDiff, shrubSamp, 0.72);
        }
    }

    vec3 result = vec3(0.0);

    result += calcDirLight(sun, norm, viewDir, baseDiff);

    for (int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; i++)
        result += calcPointLight(pointLights[i], i, norm, viewDir, baseDiff);

    for (int i = 0; i < numSpotLights && i < MAX_SPOTLIGHTS; i++)
        result += calcSpotLight(spotLights[i], i, norm, viewDir, baseDiff);

    colour = vec4(result, 1.0);
}
