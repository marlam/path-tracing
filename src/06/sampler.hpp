#pragma once

#include "math.hpp"

class Sampler
{
public:
    Sampler()
    {
    }

    // return a normalized direction vector uniformly distributed on the hemisphere
    // around the given normal vector n, computed from a uniformly distributed
    // random point in the unit sphere.
    static vec3 onUnitHemisphere(const vec3& n, const vec3& randomOnUnitSphere)
    {
        vec3 d;
        if (dot(randomOnUnitSphere, n) > 0.0f)
            d = randomOnUnitSphere;
        else
            d = -randomOnUnitSphere;
        return d;
    }
};
