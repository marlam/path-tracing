#pragma once

#include "math.hpp"

class EnvMap
{
public:
    virtual vec3 value(const vec3& /* direction */, float /* t */) const
    {
        return vec3(0.0f);
    }
};
