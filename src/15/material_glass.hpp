#pragma once

#include "material.hpp"
#include "fresnel.hpp"

class MaterialGlass : public Material
{
public:
    const vec3 absorption;
    const float refractiveIndex;

    MaterialGlass(const vec3 absorption = vec3(0.0f), float refractiveIndex = 1.5f) :
        absorption(absorption), refractiveIndex(refractiveIndex)
    {
    }

    virtual ScatterRecord scatter(const Ray& ray, const HitRecord& hr, Prng& prng) const override
    {
        vec3 attenuation = vec3(1.0f);
        float n2 = refractiveIndex;
        float n1 = 1.0f; // assume a vacuum outside the object
        if (hr.backside) {
            // the ray traveled through the volume and is now exiting:
            // swap the refractive indices and compute the absorption of radiance
            std::swap(n2, n1);
            float distInVolume = hr.a;
            attenuation = vec3(
                    std::exp(-absorption.x() * distInVolume),
                    std::exp(-absorption.y() * distInVolume),
                    std::exp(-absorption.z() * distInVolume));
        }

        vec3 refracted = refract(ray.direction, hr.normal, n1 / n2);

        bool doReflection = true;
        if (dot(refracted, refracted) > 0.0f) { // so no total internal reflection
            float cosIncident = dot(-ray.direction, hr.normal);
            float cosTransmitted = -dot(refracted, hr.normal);
            float fresnel = fresnelUnpolarized(cosIncident, cosTransmitted, n1, n2);
            doReflection = prng.in01() < fresnel;
        }

        if (doReflection) {
            vec3 reflected = reflect(ray.direction, hr.normal);
            return ScatterRecord(normalize(reflected), attenuation);
        } else {
            return ScatterRecord(normalize(refracted), attenuation);
        }
    }
};
