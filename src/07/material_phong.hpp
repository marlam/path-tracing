#pragma once

#include "material.hpp"
#include "math.hpp"
#include "sampler.hpp"
#include "texture.hpp"

class MaterialPhong : public Material
{
public:
    const Texture* k_d;
    const Texture* k_s;
    const Texture* s;

    MaterialPhong(const Texture* k_d, const Texture* k_s, const Texture* s) :
        k_d(k_d), k_s(k_s), s(s)
    {
    }

    vec3 brdf(const HitRecord& hr, const vec3& in, const vec3& out, float time) const
    {
        vec3 kd = k_d->value(hr.texcoord, time);
        vec3 ks = k_s->value(hr.texcoord, time);
        float shininess = s->value(hr.texcoord, time).x();

        vec3 diffuse = kd / pi;
        float cosRV = std::max(dot(reflect(in, hr.normal), out), 0.0f);
        vec3 specular = ks * (shininess + 2.0f) / (2.0f * pi) * pow(cosRV, shininess);
        return diffuse + specular;
    }

    virtual ScatterRecord scatter(const Ray& ray, const HitRecord& hr, Prng& prng) const override
    {
        if (hr.backside)
            return ScatterRecord();

        vec3 newDirection = Sampler::onUnitHemisphere(hr.normal, prng.onUnitSphere());
        float p = 1.0f / (2.0f * pi);
        float cosTheta = dot(hr.normal, newDirection);
        vec3 attenuation = brdf(hr, -newDirection, -ray.direction, ray.time) * cosTheta;
        return ScatterRecord(newDirection, p, attenuation);
    }
};
