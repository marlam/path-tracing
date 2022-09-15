#pragma once

#include "material.hpp"

class MaterialMirror : public Material
{
public:
    const Texture* color;

    MaterialMirror(const Texture* color) : color(color)
    {
    }

    virtual ScatterRecord scatter(const Ray& ray, const HitRecord& hr, Prng&) const override
    {
        if (hr.backside)
            return ScatterRecord();
        vec3 newDirection = normalize(reflect(ray.direction, hr.normal));
        vec3 attenuation = color->value(hr.texcoord, ray.time);
        return ScatterRecord(newDirection, attenuation);
    }
};
