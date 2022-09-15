#pragma once

#include "math.hpp"
#include "surface.hpp"

class Material
{
public:
    Material()
    {
    }

    virtual vec3 Le(const HitRecord& /* hr */, const vec3& /* out */) const
    {
        return vec3(0.0f);
    }

    virtual bool pathStopsHere() const
    {
        return false;
    }

    virtual vec3 brdf(const HitRecord& /* hr */, const vec3& /* in */, const vec3& /* out */) const
    {
        return vec3(0.0f);
    }
};
