#pragma once

#include "math.hpp"

class Ray
{
public:
    vec3 origin;
    vec3 direction; // must have unit length!
    vec3 invDirection;
    float time;

    Ray(const vec3& o, const vec3& d, float t) :
        origin(o),
        direction(d),
        invDirection(vec3(1.0f) / d),
        time(t)
    {
    }

    // return point at distance a from the origin, in given direction
    vec3 at(float a) const
    {
        return origin + a * direction;
    }
};
