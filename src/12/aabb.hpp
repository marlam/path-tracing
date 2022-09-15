#pragma once

#include "math.hpp"

class AABB
{
public:
    vec3 lo, hi;

    AABB()
    {
    }

    AABB(const vec3& lo, const vec3& hi) : lo(lo), hi(hi)
    {
    }

    vec3 center() const
    {
        return mix(lo, hi, 0.5f);
    }

    int longestAxis() const
    {
        int axis = 2;
        vec3 l = hi - lo;
        if (l.x() > l.y() && l.x() > l.z()) {
            axis = 0;
        } else if (l.y() > l.z()) {
            axis = 1;
        }
        return axis;
    }

    float surfaceArea() const
    {
        vec3 l = hi - lo;
        return 2.0f * (l.x() * l.y() + l.y() * l.z() + l.x() * l.z());
    }

    bool hit(const Ray& ray, float amin, float amax) const
    {
        for (int dim = 0; dim < 3; dim++) {
            float adimmin, adimmax;
            if (ray.invDirection[dim] < 0.0f) {
                adimmin = (hi[dim] - ray.origin[dim]) * ray.invDirection[dim];
                adimmax = (lo[dim] - ray.origin[dim]) * ray.invDirection[dim];
            } else {
                adimmin = (lo[dim] - ray.origin[dim]) * ray.invDirection[dim];
                adimmax = (hi[dim] - ray.origin[dim]) * ray.invDirection[dim];
            }
            if (adimmin > amax || amin > adimmax)
                return false;
            if (adimmin > amin)
                amin = adimmin;
            if (adimmax < amax)
                amax = adimmax;
        }
        return true;
    }
};

inline AABB merge(const AABB& aabb0, const AABB& aabb1)
{
    return AABB(vec3(std::min(aabb0.lo.x(), aabb1.lo.x()),
                     std::min(aabb0.lo.y(), aabb1.lo.y()),
                     std::min(aabb0.lo.z(), aabb1.lo.z())),
                vec3(std::max(aabb0.hi.x(), aabb1.hi.x()),
                     std::max(aabb0.hi.y(), aabb1.hi.y()),
                     std::max(aabb0.hi.z(), aabb1.hi.z())));
}

class AABBSorter
{
public:
    const std::vector<AABB>& aabbs;
    const int axis;

    AABBSorter(const std::vector<AABB>& aabbs, int axis) :
        aabbs(aabbs), axis(axis)
    {
    }

    bool operator() (unsigned int i, unsigned int j)
    {
        return (aabbs[i].center()[axis] < aabbs[j].center()[axis]);
    }
};
