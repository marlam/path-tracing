#pragma once

#include <vector>

#include "texture.hpp"
#include "sampler.hpp"

class TextureGradientNoise : public Texture
{
public:
    int width, height;
    std::vector<vec2> values;

    TextureGradientNoise(int w, int h, Prng& prng) :
        width(w), height(h), values(width * height)
    {
        for (int i = 0; i < width * height; i++)
            values[i] = Sampler::uniformOnDisk(prng.in01());
    }

    vec2 value(int x, int y) const
    {
        x = x % width;
        y = y % height;
        return values[y * width + x];
    }

    virtual vec3 value(const vec2& texcoord, float /* t */) const override
    {
        float ix = std::floor(width * texcoord.x());
        float iy = std::floor(height * texcoord.y());
        float fx = fract(width * texcoord.x());
        float fy = fract(height * texcoord.y());
        float sx = fx * fx * (3.0f - 2.0f * fx); // smoothstep function
        float sy = fy * fy * (3.0f - 2.0f * fy); // smoothstep function
        float a = mix(dot(value(ix + 0, iy + 0), vec2(fx - 0.0f, fy - 0.0f)),
                      dot(value(ix + 1, iy + 0), vec2(fx - 1.0f, fy - 0.0f)), sx);
        float b = mix(dot(value(ix + 0, iy + 1), vec2(fx - 0.0f, fy - 1.0f)),
                      dot(value(ix + 1, iy + 1), vec2(fx - 1.0f, fy - 1.0f)), sx);
        float c = mix(a, b, sy);
        return vec3(c);
    }
};
