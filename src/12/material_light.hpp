#pragma once

#include "material.hpp"

class MaterialLight : public Material
{
public:
    const vec3 radiance;

    MaterialLight(const vec3& radiance) : radiance(radiance)
    {
    }

    virtual vec3 Le(const HitRecord& hr, const vec3& /* out */) const override
    {
        return hr.backside ? vec3(0.0f) : radiance;
    }
};
