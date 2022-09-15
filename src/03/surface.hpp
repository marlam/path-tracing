#pragma once

#include "math.hpp"

class Material;

class HitRecord
{
public:
    bool haveHit;       // do we have a hit? Otherwise the following fields are irrelevant.
    float time;         // ray time
    float a;            // hit position = ray.origin + a * ray.direction
    vec3 position;      // hit position = ray.origin + a * ray.direction
    vec3 normal;        // normal at hit position; always points towards the ray, also for back sides
    vec2 texcoord;      // texture coordinates at hit position
    const Material* material; // the material of the surface

    HitRecord() : haveHit(false)
    {
    }

    HitRecord(float t, float a, const vec3& p, const vec3& n, const vec2& tc, const Material* m) :
        haveHit(true), time(t), a(a), position(p), normal(n), texcoord(tc), material(m)
    {
    }
};

class Surface
{
public:
    virtual HitRecord hit(const Ray& /* ray */, float /* amin */, float /* amax */) const
    {
        return HitRecord();
    }
};
