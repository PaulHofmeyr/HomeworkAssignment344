#version 330 core

in  vec2 uv;
out vec4 colour;

uniform sampler2D screenTexture;

// Post-process mode flags 
uniform bool nightVision; // green boost + brightness
uniform bool fisheye; // barrel distortion
uniform bool greyscale;
uniform bool inverted;
uniform bool monochrome; // map luminance to a single hue

uniform vec3 monoHue; // target hue for monochrome mode (linear colour)

// Fisheye barrel distortion
// Returns the warped UV, or vec2(-1.0) as a sentinel for "out of lens" pixels.
vec2 fisheyeUV(vec2 uv)
{
    // Map [0,1] -> [-1,1] (centred)
    vec2 p = uv * 2.0 - 1.0;
    float r = length(p);

    // Pixels beyond the unit circle are outside the fisheye lens 
    if (r > 1.0)
        return vec2(-1.0);

    // Equisolid / stereographic-style warp: pull centre outward
    float theta = asin(r); // maps r=1 -> 90°
    float strength = 1.4;
    vec2 distorted = (theta / (r + 0.0001)) * p * strength;

    return distorted * 0.5 + 0.5;
}

void main()
{
    vec2 sampleUV = uv;

    if (fisheye && !nightVision)
    {
        vec2 warped = fisheyeUV(uv);
        if (warped.x < 0.0)
        {
            // Outside the fisheye circle - render as black vignette
            colour = vec4(0.0, 0.0, 0.0, 1.0);
            return;
        }
        sampleUV = warped;
    }

    vec3 col = texture(screenTexture, sampleUV).rgb;

    if (nightVision)
    {
        // Boost brightness, apply green tint
        float lum = dot(col, vec3(0.299, 0.587, 0.114));
        lum = pow(lum * 2.5, 0.85); // boost + slight gamma
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
