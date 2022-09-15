#pragma once

#include <cmath>

#include "math.hpp"

// Pinhole camera class
class Camera
{
public:
    const float t, b, r, l;    // Frustum corners at distance=1: top, bottom, right, left

    Camera(float vfov,   // vertical field of view = vertical opening angle, in radians
            float aspectRatio) :
        t(std::tan(vfov * 0.5f)),
        b(-t),
        r(t * aspectRatio),
        l(-r)
    {
    }

    Ray getRay(float p, float q)
    {
        // get origin O and direction D in camera space
        vec3 P = vec3(mix(l, r, p), mix(b, t, q), -1.0f);
        vec3 O = vec3(0.0f);
        vec3 D = P - O;
        // create ray
        return Ray(O, normalize(D));
    }
};
