#pragma once

#include "math.hpp"

class Texture
{
public:
    virtual vec3 value(const vec2& /* texcoord */, float /* time */) const = 0;
};
