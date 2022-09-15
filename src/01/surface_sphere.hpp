#pragma once

#include "math.hpp"
#include "surface.hpp"

class SurfaceSphere : public Surface
{
private:
    HitRecord constructHitRecord(const Ray& ray, float a) const
    {
        vec3 p = ray.at(a);
        vec3 n = normalize(p - center);
        if (dot(n, -ray.direction) < 0.0f) {
            // we hit the inside of the sphere; reverse the normal
            // so that the ray continues correctly
            n = -n;
        }
        return HitRecord(a, p, n, material);
    }

public:
    const vec3 center;
    const float radius;
    const Material* material;

    SurfaceSphere(const vec3& c, float r, const Material* mat) :
        center(c), radius(r), material(mat)
    {
    }

    virtual HitRecord hit(const Ray& ray, float amin, float amax) const override
    {
        vec3 c = center;
        float r = radius;

        vec3 oc = ray.origin - c;
        float e = dot(oc, ray.direction);
        float f = dot(oc, oc) - r * r;
        float discriminant = e * e - f;

        HitRecord hr;
        if (discriminant > 0.0f) {
            float a = -e - std::sqrt(discriminant);
            if (a > amin && a < amax) {
                hr = constructHitRecord(ray, a);
            } else {
                a = -e + std::sqrt(discriminant);
                if (a > amin && a < amax) {
                    hr = constructHitRecord(ray, a);
                }
            }
        }
        return hr;
    }
};
