#pragma once

#include "math.hpp"
#include "surface.hpp"
#include "animation.hpp"

class SurfaceSphere : public Surface
{
private:
    HitRecord constructHitRecord(const Ray& ray, float a,
            const vec3& transformedCenter,
            const Transformation& T) const
    {
        vec3 p = ray.at(a);
        vec3 n = normalize(p - transformedCenter);

        // compute texture coordinates
        vec3 rn = T.rotation * n;
        float alpha = std::atan2(rn.x(), rn.z());
        float beta = std::asin(std::min(1.0f, std::max(-1.0f, rn.y())));
        float u = (alpha + pi) / (2.0f * pi);
        float v = (beta + 0.5f * pi) / pi;
        vec2 tc = vec2(u, v);

        bool backside = false;
        if (dot(n, -ray.direction) < 0.0f) {
            // we hit the inside of the sphere
            backside = true;
            n = -n;
        }

        return HitRecord(a, p, n, tc, backside, material);
    }

public:
    const vec3 center;
    const float radius;
    const Material* material;
    const Animation* animation;

    SurfaceSphere(const vec3& c, float r, const Material* mat, const Animation* anim = nullptr) :
        center(c), radius(r), material(mat), animation(anim)
    {
    }

    virtual HitRecord hit(const Ray& ray, float amin, float amax) const override
    {
        vec3 c = center;
        float r = radius;
        Transformation T;
        if (animation) {
            T = animation->at(ray.time);
            c = T * c;
            r *= T.scaling.x();
        }

        vec3 oc = ray.origin - c;
        float aq = -dot(oc, ray.direction);
        vec3 tmp = oc - dot(oc, ray.direction) * ray.direction;
        float discriminant = r * r - dot(tmp, tmp);

        HitRecord hr;
        if (discriminant > 0.0f) {
            float a1, a2;
            if (aq < 0.0f) {
                a2 = aq - std::sqrt(discriminant);
                a1 = 2.0f * aq - a2;
            } else {
                a1 = aq + std::sqrt(discriminant);
                a2 = 2.0f * aq - a1;
            }
            if (a2 > amin && a2 < amax) {
                hr = constructHitRecord(ray, a2, c, T);
            } else if (a1 > amin && a1 < amax) {
                hr = constructHitRecord(ray, a1, c, T);
            }
        }
        return hr;
    }
};
