#version 330 core

in  vec2 uv;
out vec4 colour;

uniform sampler2D screenTexture;

uniform bool nightVision;
uniform bool fisheye;
uniform bool greyscale;
uniform bool inverted;
uniform bool monochrome;

uniform vec3 monoHue;

vec2 fisheyeUV(vec2 uv, out bool outOfBounds)
{
    // Map [0,1] -> [-1,1] centred
    vec2 p = uv * 2.0 - 1.0;
    float r = length(p);

    // Outside the lens circle -> black
    if (r > 1.0)
    {
        outOfBounds = true;
        return vec2(0.0);
    }

    outOfBounds = false;

    // Barrel warp using asin so the full circle maps smoothly to the frame.
    // Avoid divide-by-zero at centre.
    float warp = (r < 0.0001) ? 1.0 : asin(r) / r;
    vec2 distorted = p * warp * 0.9;   // 0.9 keeps edges from hitting the frame border

    // Map back to [0,1]
    vec2 result = distorted * 0.5 + 0.5;

    // If the warp pushed us outside the actual texture area, show black
    // instead of clamping (which smears the edge pixels into bands).
    if (result.x < 0.0 || result.x > 1.0 || result.y < 0.0 || result.y > 1.0)
    {
        outOfBounds = true;
        return vec2(0.0);
    }

    return result;
}

void main()
{
    vec2 sampleUV = uv;

    if (fisheye && !nightVision)
    {
        bool outOfBounds;
        vec2 warped = fisheyeUV(uv, outOfBounds);
        if (outOfBounds)
        {
            colour = vec4(0.0, 0.0, 0.0, 1.0);
            return;
        }
        sampleUV = warped;
    }

    vec3 col = texture(screenTexture, sampleUV).rgb;

    if (nightVision)
    {
        float lum = dot(col, vec3(0.299, 0.587, 0.114));
        lum = pow(lum * 2.5, 0.85);
        col = vec3(lum * 0.15, lum, lum * 0.15);
    }
    else if (greyscale)
    {
        float lum = dot(col, vec3(0.2126, 0.7152, 0.0722));
        col = vec3(lum);
    }
    else if (inverted)
    {
        col = vec3(1.0) - col;
    }
    else if (monochrome)
    {
        float lum = dot(col, vec3(0.2126, 0.7152, 0.0722));
        col = monoHue * lum;
    }

    colour = vec4(col, 1.0);
}
