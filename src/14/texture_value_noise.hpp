#pragma once

#include <vector>

#include "texture.hpp"

class TextureValueNoise : public Texture
{
public:
    int width, height;
    std::vector<float> values;

    TextureValueNoise(int w, int h, Prng& prng) :
        width(w), height(h), values(width * height)
    {
        for (int i = 0; i < width * height; i++)
            values[i] = prng.in01() * 2.0f - 1.0f;
    }

    float value(int x, int y) const
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
        float a = mix(value(ix + 0, iy + 0), value(ix + 1, iy + 0), fx);
        float b = mix(value(ix + 0, iy + 1), value(ix + 1, iy + 1), fx);
        float c = mix(a, b, fy);
        return vec3(c);
    }
};
