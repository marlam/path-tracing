#pragma once

#include "math.hpp"
#include "aabb.hpp"

class Material;

class HitRecord
{
public:
    bool haveHit;       // do we have a hit? Otherwise the following fields are irrelevant.
    float a;            // hit position = ray.origin + a * ray.direction
    vec3 position;      // hit position = ray.origin + a * ray.direction
    vec3 normal;        // normal at hit position; always points towards the ray, also for back sides
    vec2 texcoord;      // texture coordinates at hit position
    bool backside;      // flag: was the hit on the backside? (the normal is flipped then)
    const Material* material; // the material of the surface

    HitRecord() : haveHit(false)
    {
    }

    HitRecord(float a, const vec3& p, const vec3& n, const vec2& tc, bool bs, const Material* m) :
        haveHit(true), a(a), position(p), normal(n), texcoord(tc), backside(bs), material(m)
    {
    }
};

class Surface
{
public:
    virtual AABB aabb(float /* t0 */, float /* t1 */) const
    {
        return AABB(vec3(0.0f), vec3(0.0f));
    }

    virtual HitRecord hit(const Ray& /* ray */, float /* amin */, float /* amax */) const
    {
        return HitRecord();
    }

    virtual vec3 direction(const vec3& /* origin */, float /* t */, Prng& /* prng */) const
    {
        return vec3(0.0f);
    }

    virtual float p(const Ray& /* ray */) const
    {
        return 0.0f;
    }
};
