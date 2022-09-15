#pragma once

#include "texture.hpp"

class TextureChecker : public Texture
{
public:
    const Texture* t0, * t1;
    int n, m;

    TextureChecker(const Texture* t0, const Texture* t1, int n, int m) :
        t0(t0), t1(t1), n(n), m(m)
    {
    }

    virtual vec3 value(const vec2& texcoord, float t) const override
    {
        int col = texcoord.x() * n;
        int row = texcoord.y() * m;
        vec3 v;
        if (row % 2 == col % 2)
            v = t0->value(texcoord, t);
        else
            v = t1->value(texcoord, t);
        return v;
    }
};
