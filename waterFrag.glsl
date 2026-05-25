#version 330 core

in vec3 fragPos;
in vec3 fragNormal;
in vec3 fragColour;
in vec2 fragUV;

out vec4 colour;

uniform vec3  viewPos;
uniform float time;          // animated — upload glfwGetTime() each frame

// ── Sun light (simplified) ────────────────────────────────────
uniform vec3 sunDir;         // normalised direction toward sun

// ── Water texture ─────────────────────────────────────────────
uniform sampler2D waterTex;  // tex unit 10
uniform bool      useWaterTex;

// ── Water parameters ──────────────────────────────────────────
// Deep colour — dark teal like the map
const vec3  WATER_DEEP    = vec3(0.05f, 0.35f, 0.55f);
// Shallow/crest colour — lighter blue
const vec3  WATER_SHALLOW = vec3(0.20f, 0.65f, 0.85f);
// Foam/specular colour
const vec3  WATER_SPEC    = vec3(0.85f, 0.92f, 1.00f);
// Transparency (fake — just darkens edges)
const float WATER_ALPHA   = 0.92f;

// ── Animated ripple ───────────────────────────────────────────
//  Two overlapping sine waves at different angles + speeds
//  give a convincing ripple without a texture file.
float ripple(vec2 uv)
{
    float s = 4.0f;   // tile scale — higher = smaller ripples
    float w1 = sin(uv.x * s       + time * 1.2f)
             * sin(uv.y * s * 0.7f + time * 0.9f);
    float w2 = sin((uv.x + uv.y) * s * 0.6f - time * 1.5f) * 0.5f;
    return (w1 + w2) * 0.5f;   // -1 .. 1
}

void main()
{
    vec3 norm    = normalize(fragNormal);
    vec3 viewDir = normalize(viewPos - fragPos);

    // ── Ripple normal perturbation ────────────────────────────
    //  Shift the surface normal slightly based on ripple height
    //  so the lighting appears to move with the waves.
    float r = ripple(fragPos.xz);
    vec3 pertNorm = normalize(norm + vec3(r * 0.12f, 0.0f, r * 0.10f));

    // ── Diffuse lighting from sun ─────────────────────────────
    vec3  lightDir = normalize(sunDir);
    float diff     = max(dot(pertNorm, lightDir), 0.0f);

    // ── Fresnel-style depth blend ─────────────────────────────
    //  Looking straight down = deep colour.
    //  Grazing angle = shallow colour.
    float fresnel  = 1.0f - max(dot(pertNorm, viewDir), 0.0f);
    fresnel        = pow(fresnel, 2.5f);
    vec3 waterCol  = mix(WATER_DEEP, WATER_SHALLOW, fresnel);

    // ── Water texture — two scrolling layers at different speeds/angles ────────
    if (useWaterTex) {
        // Layer 1: slow drift in +X+Z direction
        vec2 uv1 = fragPos.xz * 0.18f + vec2(time * 0.018f,  time * 0.012f);
        // Layer 2: faster drift at an angle, adds variety
        vec2 uv2 = fragPos.xz * 0.22f + vec2(-time * 0.010f, time * 0.022f);
        vec3 tex1 = texture(waterTex, uv1).rgb;
        vec3 tex2 = texture(waterTex, uv2).rgb;
        // Average the two layers for a natural rippling look
        vec3 texBlend = (tex1 + tex2) * 0.5f;
        // Mix texture into the procedural water colour — 55% texture, 45% procedural
        waterCol = mix(waterCol, texBlend, 0.55f);
    }

    // ── Specular highlight ────────────────────────────────────
    vec3  halfDir  = normalize(lightDir + viewDir);
    float spec     = pow(max(dot(pertNorm, halfDir), 0.0f), 64.0f);
    vec3  specCol  = WATER_SPEC * spec * 0.6f;

    // ── Ripple brightness variation ───────────────────────────
    //  Crest of waves catch more light
    float crest    = (r + 1.0f) * 0.5f;   // 0..1
    waterCol      *= 0.75f + crest * 0.35f;

    // ── Ambient ───────────────────────────────────────────────
    vec3 ambient   = waterCol * 0.35f;
    vec3 diffuse   = waterCol * diff * 0.65f;

    vec3 result    = ambient + diffuse + specCol;

    colour = vec4(result, WATER_ALPHA);
}
