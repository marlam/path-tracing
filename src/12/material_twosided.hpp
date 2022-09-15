#pragma once

#include "material.hpp"

class MaterialTwoSided : public Material
{
private:
    static HitRecord toFront(const HitRecord& hr)
    {
        // the normal already points outwards; we don't have to flip it again
        return HitRecord(hr.a, hr.position, hr.normal, hr.texcoord, false, hr.surface, hr.material);
    }

public:
    const Material* front;
    const Material* back;

    MaterialTwoSided(const Material* f, const Material* b) :
        front(f), back(b)
    {
    }

    virtual vec3 Le(const HitRecord& hr, const vec3& out) const override
    {
        if (hr.backside)
            return back->Le(toFront(hr), out);
        else
            return front->Le(hr, out);
    }

    virtual ScatterRecord scatter(const Ray& ray, const HitRecord& hr, Prng& prng) const override
    {
        if (hr.backside)
            return back->scatter(ray, toFront(hr), prng);
        else
            return front->scatter(ray, hr, prng);
    }
};
