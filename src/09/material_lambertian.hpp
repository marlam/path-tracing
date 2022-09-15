#pragma once

#include "material.hpp"
#include "math.hpp"
#include "sampler.hpp"
#include "texture.hpp"
#include "tangentspace.hpp"

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

        TangentSpace ts(hr.normal);
        vec3 newDirectionTS = Sampler::cosineWeightedOnHemisphere(prng.in01(), prng.in01());
        vec3 newDirection = ts.toWorldSpace(newDirectionTS);
        float cosTheta = dot(hr.normal, newDirection);
        if (cosTheta <= 0.0f)
            return ScatterRecord();
        float p = cosTheta / pi;
        vec3 attenuation = brdf(hr, ray.time) * cosTheta;
        return ScatterRecord(newDirection, p, attenuation);
    }
};
