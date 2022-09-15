#pragma once

#include "material.hpp"
#include "math.hpp"
#include "texture.hpp"

class MaterialLambertian : public Material
{
public:
    const Texture* albedo;

    MaterialLambertian(const Texture* albedo) : albedo(albedo)
    {
    }

    virtual vec3 brdf(const HitRecord& hr, const vec3& /* in */, const vec3& /* out */) const override
    {
        vec3 a = albedo->value(hr.texcoord, hr.time);
        return a / pi;
    }
};
