#pragma once

#include <vector>

#include "texture.hpp"
#include "math.hpp"

class TextureWorleyNoise : public Texture
{
public:
    std::vector<vec2> points;

    TextureWorleyNoise(int n, Prng& prng) : points(n)
    {
        for (int i = 0; i < n; i++)
            points[i] = vec2(prng.in01(), prng.in01());
    }

    virtual vec3 value(const vec2& texcoord, float /* t */) const override
    {
        vec2 uv = vec2(fract(texcoord.x()), fract(texcoord.y()));
        float d1 = std::numeric_limits<float>::max();
        float d2 = std::numeric_limits<float>::max();
        float d3 = std::numeric_limits<float>::max();
        for (size_t i = 0; i < points.size(); i++) {
            float d = length(uv - points[i]);
            if (d < d1) {
                d3 = d2;
                d2 = d1;
                d1 = d;
            } else if (d < d2) {
                d3 = d2;
                d2 = d;
            } else if (d < d3) {
                d3 = d;
            }
        }
        return vec3(d1, d2, d3);
    }
};
