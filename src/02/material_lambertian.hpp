#pragma once

#include "material.hpp"
#include "math.hpp"

class MaterialLambertian : public Material
{
public:
    const vec3 albedo;

    MaterialLambertian(const vec3& albedo) : albedo(albedo)
    {
    }

    virtual vec3 brdf(const HitRecord& /* hr */, const vec3& /* in */, const vec3& /* out */) const override
    {
        return albedo / pi;
    }
};
