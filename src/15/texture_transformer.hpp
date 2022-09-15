#pragma once

#include "texture.hpp"

class TextureTransformer : public Texture
{
public:
    const Texture* tex;
    const vec2 factor, offset;

    TextureTransformer(const Texture* tex, const vec2& factor, const vec2& offset) :
        tex(tex), factor(factor), offset(offset)
    {
    }

    virtual vec3 value(const vec2& texcoord, float t) const override
    {
        return tex->value(factor * texcoord + offset, t);
    }
};
