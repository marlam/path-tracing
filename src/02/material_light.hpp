#pragma once

#include "material.hpp"

class MaterialLight : public Material
{
public:
    const vec3 radiance;

    MaterialLight(const vec3& radiance) : radiance(radiance)
    {
    }

    virtual vec3 Le(const HitRecord& /* hr */, const vec3& /* out */) const override
    {
        return radiance;
    }

    virtual bool pathStopsHere() const override
    {
        return true;
    }
};
