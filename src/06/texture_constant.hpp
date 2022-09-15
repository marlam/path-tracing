#pragma once

#include "texture.hpp"

class TextureConstant : public Texture
{
public:
    const vec3 val;

    TextureConstant(const vec3& v) : val(v)
    {
    }

    virtual vec3 value(const vec2& /* texcoord */, float /* time */) const override
    {
        return val;
    }
};
