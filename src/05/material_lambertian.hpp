#pragma once

#include "material.hpp"
#include "math.hpp"
#include "sampler.hpp"
#include "texture.hpp"

class MaterialLambertian : public Material
{
public:
    const Texture* albedo;

    MaterialLambertian(const Texture* albedo) : albedo(albedo)
    {
    }

    vec3 brdf(const HitRecord& hr, float time) const
    {
        vec3 a = albedo->value(hr.texcoord, time);
        return a / pi;
    }

    virtual ScatterRecord scatter(const Ray& ray, const HitRecord& hr, Prng& prng) const override
    {
        if (hr.backside)
            return ScatterRecord();

        vec3 newDirection = Sampler::onUnitHemisphere(hr.normal, prng.onUnitSphere());
        float p = 1.0f / (2.0f * pi);
        float cosTheta = dot(hr.normal, newDirection);
        vec3 attenuation = brdf(hr, ray.time) * cosTheta;
        return ScatterRecord(newDirection, p, attenuation);
    }
};
