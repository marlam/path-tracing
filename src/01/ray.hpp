#pragma once

#include "math.hpp"

class Ray
{
public:
    vec3 origin;
    vec3 direction; // must have unit length!

    Ray(const vec3& o, const vec3& d) :
        origin(o),
        direction(d)
    {
    }

    // return point at distance a from the origin, in given direction
    vec3 at(float a) const
    {
        return origin + a * direction;
    }
};
