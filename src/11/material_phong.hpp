#pragma once

#include "material.hpp"
#include "math.hpp"
#include "sampler.hpp"
#include "texture.hpp"
#include "tangentspace.hpp"

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

    vec3 brdf(const vec3& n, const vec3& l, const vec3& v,
            const vec3& kd, const vec3& ks, float shininess) const
    {
        vec3 diffuse = kd / pi;
        float cosRV = std::max(dot(reflect(-l, n), v), 0.0f);
        vec3 specular = ks * (shininess + 2.0f) / (2.0f * pi) * pow(cosRV, shininess);
        return diffuse + specular;
    }

    virtual ScatterRecord scatter(const Ray& ray, const HitRecord& hr, Prng& prng) const override
    {
        if (hr.backside)
            return ScatterRecord();

        vec3 kd = k_d->value(hr.texcoord, ray.time);
        vec3 ks = k_s->value(hr.texcoord, ray.time);
        float shininess = s->value(hr.texcoord, ray.time).x();

        vec3 n = hr.normal;
        vec3 v = -ray.direction;
        vec3 r = reflect(ray.direction, n);

        float sumKd = kd.x() + kd.y() + kd.z();
        float sumKs = ks.x() + ks.y() + ks.z();
        float sum = sumKd + sumKs + 1e-4f /* so that sum > 0 */;
        float specularProbability = sumKs / sum;
        if (specularProbability < 0.1f)
            specularProbability = 0.1f;
        else if (specularProbability > 0.9f)
            specularProbability = 0.9f;

        vec3 l; // this is the new direction
        if (prng.in01() < specularProbability) {
            // act specular
            vec3 newDirectionAroundR = Sampler::phongWeightedOnHemisphere(shininess, prng.in01(), prng.in01());
            TangentSpace rts = TangentSpace(r);
            l = normalize(rts.toWorldSpace(newDirectionAroundR));
        } else {
            // act diffuse
            TangentSpace ts(n);
            vec3 newDirectionTS = Sampler::cosineWeightedOnHemisphere(prng.in01(), prng.in01());
            l = normalize(ts.toWorldSpace(newDirectionTS));
        }

        float cosTheta = dot(l, n);
        if (cosTheta <= 0.0f)
            return ScatterRecord();

        float diffusePdfValue = cosTheta / pi;
        float specularPdfValue = 0.5f / pi * (shininess + 1.0f)
            * std::pow(std::max(dot(r, l), 0.0f), shininess);
        float p = mix(diffusePdfValue, specularPdfValue, specularProbability);

        vec3 attenuation = brdf(n, l, v, kd, ks, shininess) * cosTheta;
        return ScatterRecord(l, p, attenuation);
    }

    virtual ScatterRecord scatterToDirection(const Ray& ray, const HitRecord& hr, const vec3& direction) const override
    {
        float cosTheta = dot(hr.normal, direction);
        if (cosTheta <= 0.0f)
            return ScatterRecord();

        float p = cosTheta / pi;
        vec3 kd = k_d->value(hr.texcoord, ray.time);
        vec3 ks = k_s->value(hr.texcoord, ray.time);
        float shininess = s->value(hr.texcoord, ray.time).x();
        vec3 attenuation = brdf(hr.normal, direction, -ray.direction, kd, ks, shininess) * cosTheta;
        return ScatterRecord(direction, p, attenuation);
    }
};
