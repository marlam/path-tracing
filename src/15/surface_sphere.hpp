#pragma once

#include "math.hpp"
#include "surface.hpp"
#include "animation.hpp"
#include "tangentspace.hpp"

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

        // compute tangent
        vec3 t = vec3(std::cos(alpha), 0.0f, -std::sin(alpha));

        bool backside = false;
        if (dot(n, -ray.direction) < 0.0f) {
            // we hit the inside of the sphere
            backside = true;
            n = -n;
        }

        return HitRecord(a, p, n, tc, t, backside, this, material);
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

    virtual AABB aabb(float t0, float t1) const override
    {
        if (!animation) {
            return AABB(center - vec3(radius), center + vec3(radius));
        } else {
            Transformation T0 = animation->at(t0);
            vec3 c0 = T0 * center;
            float r0 = T0.scaling.x() * radius;
            AABB box(c0 - vec3(r0), c0 + vec3(r0));
            const int steps = 16;
            for (int i = 1; i < steps; i++) {
                float t = mix(t0, t1, i / (steps - 1.0f)); // end at t1!
                Transformation T = animation->at(t);
                vec3 c = T * center;
                float r = T.scaling.x() * radius;
                box = merge(box, AABB(c - vec3(r), c + vec3(r)));
            }
            return box;
        }
    }

    HitRecord hit(const vec3& c, float r, const Transformation& T, const Ray& ray, float amin, float amax) const
    {
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

    void getCR(float t, vec3& c, float& r, Transformation& T) const
    {
        c = center;
        r = radius;
        if (animation) {
            T = animation->at(t);
            c = T * c;
            r *= T.scaling.x();
        }
    }

    virtual HitRecord hit(const Ray& ray, float amin, float amax) const override
    {
        vec3 c;
        float r;
        Transformation T;
        getCR(ray.time, c, r, T);
        return hit(c, r, T, ray, amin, amax);
    }

    virtual vec3 direction(const vec3& origin, float t, Prng& prng) const override
    {
        vec3 c;
        float r;
        Transformation T;
        getCR(t, c, r, T);

        vec3 dir;
        vec3 cmo = c - origin;
        float distanceSquared = dot(cmo, cmo);
        float radiusSquared = r * r;
        if (distanceSquared <= radiusSquared) {
            // We are inside the sphere. Any direction will hit the sphere.
            dir = Sampler::uniformOnSphere(prng.in01(), prng.in01());
        } else {
            // We are outside the sphere. Generate a direction that hits it.
            float discriminant = 1.0f - radiusSquared / distanceSquared;
            float cosThetaMax = std::sqrt(std::max(0.0f, discriminant));
            vec3 vectorAroundCmo = Sampler::uniformTowardsSphere(cosThetaMax, prng.in01(), prng.in01());
            dir = normalize(TangentSpace(normalize(cmo)).toWorldSpace(vectorAroundCmo));
        }
        return dir;
    }

    virtual float p(const Ray& ray) const override
    {
        vec3 c;
        float r;
        Transformation T;
        getCR(ray.time, c, r, T);

        float v = 0.0f;
        vec3 cmo = c - ray.origin;
        float distanceSquared = dot(cmo, cmo);
        float radiusSquared = r * r;
        if (distanceSquared <= radiusSquared) {
            // We are inside the sphere. The solid angle is thus 4pi,
            // and any direction will hit the sphere.
            v = 1.0f / (4.0f * pi);
        } else {
            // We are outside the sphere. The sphere area is therefore
            // visible as a circle with a solid angle of at most 2pi.
            HitRecord hr = hit(c, r, T, ray, 0.0f, std::numeric_limits<float>::max());
            if (hr.haveHit) {
                float discriminant = 1.0f - radiusSquared / distanceSquared;
                float cosThetaMax = std::sqrt(std::max(0.0f, discriminant));
                float solidAngle = 2.0f * pi * (1.0f - cosThetaMax);
                v = 1.0f / solidAngle;
            }
        }
        return v;
    }
};
